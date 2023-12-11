#include "shadowMapper.h"
#include "camera.h"
#include "collections.h"
#include "gfx/gl/shaders/vs-shadowDynamicPoint.h"
#include "gfx/gl/shaders/vs-shadowDynamicPointInst.h"
#include "gfx/gl/shaders/vs-shadowFixedPoint.h"
#include "location.h"
#include "maths.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include "sorting.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <tuple>
#include <vector>

ShadowMapper::ShadowMapper(const TextureAbsCoord & s) :
	fixedPoint {shadowFixedPoint_vs}, dynamicPointInst {shadowDynamicPointInst_vs}, size {s}
{
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static constexpr RGBA border {std::numeric_limits<RGBA::value_type>::infinity()};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border));

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

constexpr std::array<std::array<TextureAbsRegion, ShadowMapper::SHADOW_BANDS>, ShadowMapper::SHADOW_BANDS> viewports {{
		{{
				{31, 31, 0, 0}, // full
		}},
		{{
				{31, 31, 0, 1}, // lower half
				{31, 1, 0, 1}, // upper half
		}},
		{{
				{31, 31, 0, 1}, // lower half
				{31, 1, 1, 1}, // upper left
				{1, 1, 1, 1}, // upper right
		}},
		{{
				{31, 31, 1, 1}, // lower left
				{1, 31, 1, 1}, // lower right
				{31, 1, 1, 1}, // upper left
				{1, 1, 1, 1}, // upper right
		}},
}};
constexpr std::array<std::array<TextureRelRegion, ShadowMapper::SHADOW_BANDS>, ShadowMapper::SHADOW_BANDS>
		shadowMapRegions {{
				{{
						{0.5F, 0.5F, 0.5F, 0.5F}, // full
				}},
				{{
						{0.5F, 0.25F, 0.5F, 0.25F}, // lower half
						{0.5F, 0.25F, 0.5F, 0.75F}, // upper half
				}},
				{{
						{0.5F, 0.25F, 0.5F, 0.25F}, // lower half
						{0.25F, 0.25F, 0.25F, 0.75F}, // upper left
						{0.25F, 0.25F, 0.75F, 0.75F}, // upper right
				}},

				{{
						{0.25F, 0.25F, 0.25F, 0.25F}, // lower left
						{0.25F, 0.25F, 0.75F, 0.25F}, // lower right
						{0.25F, 0.25F, 0.25F, 0.75F}, // upper left
						{0.25F, 0.25F, 0.75F, 0.75F}, // upper right
				}},
		}};
constexpr std::array<GlobalDistance, ShadowMapper::SHADOW_BANDS + 1> shadowBands {
		1000,
		250000,
		750000,
		2500000,
		10000000,
};
static_assert(viewports.size() == shadowMapRegions.size());
static_assert(shadowBands.size() == shadowMapRegions.size() + 1);

struct DefinitionsInserter {
	auto
	operator++()
	{
		return out.maps++;
	};

	auto
	operator*()
	{
		return std::tie(out.projections[out.maps], out.regions[out.maps]);
	}

	ShadowMapper::Definitions & out;
};

std::vector<std::array<Position3D, 4>>
ShadowMapper::getBandViewExtents(const Camera & camera, const glm::mat4 & lightView)
{
	std::vector<std::array<Position3D, 4>> bandViewExtents;
	for (const auto dist : shadowBands) {
		const auto extents = camera.extentsAtDist(dist);
		bandViewExtents.emplace_back(extents * [&lightView](const auto & e) -> Position3D {
			return lightView * glm::vec4(Position3D {e}, 1);
		});
		if (std::none_of(extents.begin(), extents.end(), [targetDist = dist - 1](const auto & e) {
				return e.w > targetDist;
			})) {
			break;
		}
	}
	return bandViewExtents;
}

ShadowMapper::Definitions
ShadowMapper::update(const SceneProvider & scene, const Direction3D & dir, const Camera & camera) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	const auto lightViewDir = glm::lookAt(origin, dir, up);
	const auto lightView = lightViewDir * glm::translate(RelativePosition3D {-camera.getPosition()});
	const auto lightViewPoint = camera.getPosition();
	const auto bandViewExtents = getBandViewExtents(camera, lightView);

	Definitions out;
	std::transform(bandViewExtents.begin(), std::prev(bandViewExtents.end()), std::next(bandViewExtents.begin()),
			DefinitionsInserter {out},
			[&scene, this, &lightView, bands = bandViewExtents.size() - 2, &out, &lightViewPoint, &lightViewDir](
					const auto & near, const auto & far) {
				const auto extents_minmax = [extents = std::span {near.begin(), far.end()}](auto && comp) {
					const auto mm = std::minmax_element(extents.begin(), extents.end(), comp);
					return std::make_pair(comp.get(*mm.first), comp.get(*mm.second));
				};

				const auto lightProjection = [](const auto & x, const auto & y, const auto & z) {
					return glm::ortho(x.first, x.second, y.first, y.second, -z.second, -z.first);
				}(extents_minmax(CompareBy {0}), extents_minmax(CompareBy {1}), extents_minmax(CompareBy {2}));

				const auto lightViewDirProjection = lightProjection * lightViewDir;
				fixedPoint.setViewProjection(lightViewPoint, lightViewDirProjection);
				dynamicPoint.setViewProjection(lightViewPoint, lightViewDirProjection);
				dynamicPointInst.setViewProjection(lightViewPoint, lightViewDirProjection);

				const auto & viewport = viewports[bands][out.maps];
				glViewport(size.x >> viewport.x, size.y >> viewport.y, size.x >> viewport.z, size.y >> viewport.w);
				scene.shadows(*this);

				return std::make_pair(lightProjection * lightView, shadowMapRegions[bands][out.maps]);
			});

	glCullFace(GL_BACK);

	return out;
}

ShadowMapper::FixedPoint::FixedPoint(const Shader & vs) :
	Program {vs}, viewProjectionLoc {*this, "viewProjection"}, viewPointLoc {*this, "viewPoint"}
{
}

void
ShadowMapper::FixedPoint::setViewProjection(const GlobalPosition3D viewPoint, const glm::mat4 & viewProjection) const
{
	use();
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
	glUniform3iv(viewPointLoc, 1, glm::value_ptr(viewPoint));
}

void
ShadowMapper::FixedPoint::use() const
{
	glUseProgram(*this);
}

ShadowMapper::DynamicPoint::DynamicPoint() :
	Program {shadowDynamicPoint_vs}, viewProjectionLoc {*this, "viewProjection"}, viewPointLoc {*this, "viewPoint"},
	modelLoc {*this, "model"}, modelPosLoc {*this, "modelPos"}
{
}

void
ShadowMapper::DynamicPoint::setViewProjection(const GlobalPosition3D viewPoint, const glm::mat4 & viewProjection) const
{
	glUseProgram(*this);
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
	glUniform3iv(viewPointLoc, 1, glm::value_ptr(viewPoint));
}

void
ShadowMapper::DynamicPoint::use(const Location & location) const
{
	glUseProgram(*this);
	setModel(location);
}

void
ShadowMapper::DynamicPoint::setModel(const Location & location) const
{
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rotate_ypr(location.rot)));
	glUniform3iv(modelPosLoc, 1, glm::value_ptr(location.pos));
}
