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

ShadowMapper::ShadowMapper(const glm::ivec2 & s) : size {s}
{
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

glm::mat4x4
ShadowMapper::update(const SceneProvider & scene, const glm::vec3 & dir, const Camera & camera) const
{
	constexpr auto BACK_OFF = 3600.f;
	const glm::vec3 range = glm::normalize(dir) * BACK_OFF;
	auto viewExtents = camera.extentsAtDist(1) + camera.extentsAtDist(1000);
	for (auto & e : viewExtents) {
		e = glm::round(e);
	}
	const auto extents_minmax = [&viewExtents](auto && comp) {
		const auto mm = std::minmax_element(viewExtents.begin(), viewExtents.end(), comp);
		return std::make_pair(comp.get(*mm.first), comp.get(*mm.second));
	};
	// Find camera view centre
	const auto centre = [](const auto & x, const auto & y, const auto & z) {
		return glm::vec3 {midpoint(x), midpoint(y), midpoint(z)};
	}(extents_minmax(CompareBy {0}), extents_minmax(CompareBy {1}), extents_minmax(CompareBy {2}));

	const auto lightView = glm::lookAt(centre, centre + range, up);
	for (auto & e : viewExtents) {
		e = lightView * glm::vec4(e, 1);
	}

	const auto lightProjection = [](const auto & x, const auto & y, const auto & z) {
		return glm::ortho(x.first, x.second, y.first, y.second, z.first, z.second);
	}(extents_minmax(CompareBy {0}), extents_minmax(CompareBy {1}), extents_minmax(CompareBy {2}));

	const auto lightViewProjection = lightProjection * lightView;
	fixedPoint.setViewProjection(lightViewProjection);
	dynamicPoint.setViewProjection(lightViewProjection);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glViewport(0, 0, size.x, size.y);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	scene.shadows(*this);
	glCullFace(GL_BACK);

	return lightViewProjection;
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