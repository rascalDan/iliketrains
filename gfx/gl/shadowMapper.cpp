#include "shadowMapper.h"
#include "camera.h"
#include "collections.h"
#include "gfx/gl/shaders/gs-commonShadowPoint.h"
#include "gfx/gl/shaders/vs-shadowDynamicPoint.h"
#include "gfx/gl/shaders/vs-shadowDynamicPointInst.h"
#include "gfx/gl/shaders/vs-shadowFixedPoint.h"
#include "gl_traits.h"
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
	fixedPoint {shadowFixedPoint_vs, commonShadowPoint_gs},
	dynamicPointInst {shadowDynamicPointInst_vs, commonShadowPoint_gs}, size {s}
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
	glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, size.x, size.y, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static constexpr RGBA border {std::numeric_limits<RGBA::value_type>::infinity()};
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
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

std::vector<std::array<RelativePosition3D, 4>>
ShadowMapper::getBandViewExtents(const Camera & camera, const glm::mat4 & lightViewDir)
{
	std::vector<std::array<RelativePosition3D, 4>> bandViewExtents;
	for (const auto dist : shadowBands) {
		const auto extents = camera.extentsAtDist(dist);
		bandViewExtents.emplace_back(extents * [&lightViewDir, cameraPos = camera.getPosition()](const auto & e) {
			return glm::mat3(lightViewDir) * (e.xyz() - cameraPos);
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
	glViewport(0, 0, size.x, size.y);

	const auto lightViewDir = glm::lookAt({}, dir, up);
	const auto lightViewPoint = camera.getPosition();
	const auto bandViewExtents = getBandViewExtents(camera, lightViewDir);
	Definitions out;
	std::transform(bandViewExtents.begin(), std::prev(bandViewExtents.end()), std::next(bandViewExtents.begin()),
			DefinitionsInserter {out},
			[this, bands = bandViewExtents.size() - 2, &out, &lightViewDir](
					const auto & near, const auto & far) mutable {
				const auto extents_minmax = [extents = std::span {near.begin(), far.end()}](auto && comp) {
					const auto mm = std::minmax_element(extents.begin(), extents.end(), comp);
					return std::make_pair(comp.get(*mm.first), comp.get(*mm.second));
				};

				const auto lightProjection = [](const auto & x, const auto & y, const auto & z) {
					return glm::ortho(x.first, x.second, y.first, y.second, -z.second, -z.first);
				}(extents_minmax(CompareBy {0}), extents_minmax(CompareBy {1}), extents_minmax(CompareBy {2}));

				const auto lightViewDirProjection = lightProjection * lightViewDir;
				fixedPoint.setViewProjection(lightViewDirProjection, out.maps);
				dynamicPoint.setViewProjection(lightViewDirProjection, out.maps);
				dynamicPointInst.setViewProjection(lightViewDirProjection, out.maps);

				return std::make_pair(lightViewDirProjection, shadowMapRegions[0][0]);
			});
	fixedPoint.setViewPoint(lightViewPoint, out.maps);
	dynamicPoint.setViewPoint(lightViewPoint, out.maps);
	dynamicPointInst.setViewPoint(lightViewPoint, out.maps);
	scene.shadows(*this);

	glCullFace(GL_BACK);

	return out;
}

ShadowMapper::FixedPoint::FixedPoint(const Shader & vs, const Shader & gs) :
	Program {vs, gs}, viewProjectionLoc {{
							  {*this, "viewProjection[0]"},
							  {*this, "viewProjection[1]"},
							  {*this, "viewProjection[2]"},
							  {*this, "viewProjection[3]"},
					  }},
	viewProjectionsLoc {*this, "viewProjections"}, viewPointLoc {*this, "viewPoint"}
{
}

void
ShadowMapper::FixedPoint::setViewPoint(const GlobalPosition3D viewPoint, size_t n) const
{
	use();
	glUniform(viewPointLoc, viewPoint);
	glUniform(viewProjectionsLoc, static_cast<GLint>(n));
}

void
ShadowMapper::FixedPoint::setViewProjection(const glm::mat4 & viewProjection, size_t n) const
{
	use();
	glUniform(viewProjectionLoc[n], viewProjection);
}

void
ShadowMapper::FixedPoint::use() const
{
	glUseProgram(*this);
}

ShadowMapper::DynamicPoint::DynamicPoint() :
	Program {shadowDynamicPoint_vs, commonShadowPoint_gs}, viewProjectionLoc {{
																   {*this, "viewProjection[0]"},
																   {*this, "viewProjection[1]"},
																   {*this, "viewProjection[2]"},
																   {*this, "viewProjection[3]"},
														   }},
	viewProjectionsLoc {*this, "viewProjections"}, viewPointLoc {*this, "viewPoint"}, modelLoc {*this, "model"},
	modelPosLoc {*this, "modelPos"}
{
}

void
ShadowMapper::DynamicPoint::setViewPoint(const GlobalPosition3D viewPoint, size_t n) const
{
	glUseProgram(*this);
	glUniform(viewPointLoc, viewPoint);
	glUniform(viewProjectionsLoc, static_cast<GLint>(n));
}

void
ShadowMapper::DynamicPoint::setViewProjection(const glm::mat4 & viewProjection, size_t n) const
{
	glUseProgram(*this);
	glUniform(viewProjectionLoc[n], viewProjection);
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
	glUniform(modelLoc, location.getRotationTransform());
	glUniform(modelPosLoc, location.pos);
}
