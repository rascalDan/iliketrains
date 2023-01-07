#include "shadowMapper.h"
#include "camera.h"
#include "collections.hpp"
#include "gfx/gl/shaders/vs-shadowDynamicPoint.h"
#include "gfx/gl/shaders/vs-shadowFixedPoint.h"
#include "location.hpp"
#include "maths.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include "sorting.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <tuple>
#include <vector>

ShadowMapper::ShadowMapper(const glm::ivec2 & s) : size {s}
{
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static constexpr glm::vec4 border {std::numeric_limits<float>::infinity()};
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

constexpr std::array<glm::ivec4, ShadowMapper::SHADOW_BANDS> viewports {{
		{31, 31, 1, 1},
		{1, 31, 1, 1},
		{31, 1, 1, 1},
		{1, 1, 1, 1},
}};
constexpr std::array<glm::vec4, ShadowMapper::SHADOW_BANDS> shadowMapRegions {{
		{0.25F, 0.25F, 0.25F, 0.25F},
		{0.25F, 0.25F, 0.75F, 0.25F},
		{0.25F, 0.25F, 0.25F, 0.75F},
		{0.25F, 0.25F, 0.75F, 0.75F},
}};
constexpr std::array<float, ShadowMapper::SHADOW_BANDS + 1> shadowBands {
		1.F,
		250.F,
		750.F,
		2500.F,
		10000.F,
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

ShadowMapper::Definitions
ShadowMapper::update(const SceneProvider & scene, const glm::vec3 & dir, const Camera & camera) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glViewport(0, 0, size.x, size.y);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	std::vector<std::array<glm::vec3, 4>> bandViewExtents;
	for (const auto dist : shadowBands) {
		const auto extents = camera.extentsAtDist(dist);
		bandViewExtents.emplace_back(extents * [](const auto & e) -> glm::vec3 {
			return e;
		});
		if (std::none_of(extents.begin(), extents.end(), [targetDist = dist * 0.99F](const glm::vec4 & e) {
				return e.w > targetDist;
			})) {
			break;
		}
	}

	const auto lightView = glm::lookAt(camera.getPosition(), camera.getPosition() + dir, up);
	for (auto & band : bandViewExtents) {
		for (auto & e : band) {
			e = lightView * glm::vec4(e, 1);
		}
	}

	Definitions out;
	std::transform(bandViewExtents.begin(), std::prev(bandViewExtents.end()), std::next(bandViewExtents.begin()),
			DefinitionsInserter {out},
			[&scene, this, &lightView, band = 0U](const auto & near, const auto & far) mutable {
				const auto extents_minmax = [extents = std::span {near.begin(), far.end()}](auto && comp) {
					const auto mm = std::minmax_element(extents.begin(), extents.end(), comp);
					return std::make_pair(comp.get(*mm.first), comp.get(*mm.second));
				};

				const auto lightProjection = [](const auto & x, const auto & y, const auto & z) {
					return glm::ortho(x.first, x.second, y.first, y.second, -z.second, -z.first);
				}(extents_minmax(CompareBy {0}), extents_minmax(CompareBy {1}), extents_minmax(CompareBy {2}));

				const auto lightViewProjection = lightProjection * lightView;
				fixedPoint.setViewProjection(lightViewProjection);
				dynamicPoint.setViewProjection(lightViewProjection);

				const auto & viewport = viewports[band];
				glViewport(size.x >> viewport.x, size.y >> viewport.y, size.x >> viewport.z, size.y >> viewport.w);
				scene.shadows(*this);

				return std::make_pair(lightViewProjection, shadowMapRegions[band++]);
			});

	glCullFace(GL_BACK);

	return out;
}

ShadowMapper::FixedPoint::FixedPoint() : Program {shadowFixedPoint_vs}, viewProjectionLoc {*this, "viewProjection"} { }
void
ShadowMapper::FixedPoint::setViewProjection(const glm::mat4 & viewProjection) const
{
	use();
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
}
void
ShadowMapper::FixedPoint::use() const
{
	glUseProgram(*this);
}

ShadowMapper::DynamicPoint::DynamicPoint() :
	Program {shadowDynamicPoint_vs}, viewProjectionLoc {*this, "viewProjection"}, modelLoc {*this, "model"}
{
}
void
ShadowMapper::DynamicPoint::setViewProjection(const glm::mat4 & viewProjection) const
{
	glUseProgram(*this);
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
}
void
ShadowMapper::DynamicPoint::use(const Location & location) const
{
	glUseProgram(*this);
	const auto model = glm::translate(location.pos) * rotate_ypr(location.rot);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}
