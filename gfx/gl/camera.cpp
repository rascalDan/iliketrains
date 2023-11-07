#include "camera.h"
#include <collections.h>
#include <glm/gtx/intersect.hpp> // IWYU pragma: keep
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <maths.h>
#include <ray.h>

Camera::Camera(Position3D pos, Angle fov, Angle aspect, Distance zNear, Distance zFar) :
	position {pos}, forward {::north}, up {::up}, near {zNear}, far {zFar},
	projection {glm::perspective(fov, aspect, zNear, zFar)},
	viewProjection {projection * glm::lookAt(position, position + forward, up)},
	inverseViewProjection {glm::inverse(viewProjection)}
{
}

Ray
Camera::unProject(const ScreenRelCoord & mouse) const
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

std::array<glm::vec4, 4>
Camera::extentsAtDist(const float dist) const
{
	const auto clampToSeaFloor = [this, dist](const glm::vec3 & target) {
		if (target.z < -1.5F) {
			const auto vec = glm::normalize(target - position);
			constexpr glm::vec3 seafloor {0, 0, -1.5F};
			float outdist;
			if (glm::intersectRayPlane(position, vec, seafloor, ::up, outdist)) {
				return (vec * outdist + position) ^ outdist;
			}
		}
		return target ^ dist;
	};
	const auto depth = -(2.F * (dist - near) * far) / (dist * (near - far)) - 1.F;
	static constexpr const std::array extents {-1.F, 1.F};
	static constexpr const auto cartesianExtents = extents * extents;
	return cartesianExtents * [&depth, this, &clampToSeaFloor](const auto & extent) {
		const glm::vec4 in {extent.first, extent.second, depth, 1.F};
		return clampToSeaFloor(perspective_divide(inverseViewProjection * in));
	};
}
