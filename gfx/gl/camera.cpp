#include "camera.h"
#include <collections.hpp>
#include <glm/gtx/intersect.hpp> // IWYU pragma: keep
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <maths.h>
#include <ray.hpp>

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar) :
	position {pos}, forward {::north}, up {::up}, fov {fov}, aspect {aspect}, near {zNear}, far {zFar},
	projection {glm::perspective(fov, aspect, zNear, zFar)}
{
	updateView();
}

Ray
Camera::unProject(const glm::vec2 & mouse) const
{
	static constexpr const glm::vec4 screen {0, 0, 1, 1};
	const auto mouseProjection = glm::lookAt(::origin, forward, up);
	return {position, glm::normalize(glm::unProject(mouse ^ 1, mouseProjection, projection, screen))};
}

void
Camera::updateView()
{
	viewProjection = projection * glm::lookAt(position, position + forward, up);
	inverseViewProjection = glm::inverse(viewProjection);
}

glm::vec3
Camera::upFromForward(const glm::vec3 & forward)
{
	const auto right = glm::cross(forward, ::down);
	return glm::cross(forward, right);
}

std::array<glm::vec3, 4>
Camera::extentsAtDist(const float dist) const
{
	const auto clampToSeaFloor = [this](const glm::vec3 & target) {
		if (target.z < -1.5f) {
			const auto vec = glm::normalize(target - position);
			constexpr glm::vec3 seafloor {0, 0, -1.5};
			float outdist;
			if (glm::intersectRayPlane(position, vec, seafloor, ::up, outdist)) {
				return vec * outdist + position;
			}
		}
		return target;
	};
	const auto depth = -(2.f * (dist - near) * far) / (dist * (near - far)) - 1.f;
	static constexpr const std::array extents {-1.F, 1.F};
	static constexpr const auto cartesianExtents = extents * extents;
	return cartesianExtents * [&depth, this, &clampToSeaFloor](const auto & extent) {
		const glm::vec4 in {extent.first, extent.second, depth, 1.f};
		return clampToSeaFloor(perspective_divide(inverseViewProjection * in));
	};
}
