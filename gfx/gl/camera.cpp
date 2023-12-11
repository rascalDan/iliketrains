#include "camera.h"
#include <collections.h>
#include <glm/gtx/transform.hpp>
#include <maths.h>
#include <ray.h>

Camera::Camera(GlobalPosition3D pos, Angle fov, Angle aspect, GlobalDistance zNear, GlobalDistance zFar) :
	position {pos}, forward {::north}, up {::up}, near {zNear}, far {zFar},
	projection {
			glm::perspective(fov, aspect, static_cast<RelativeDistance>(zNear), static_cast<RelativeDistance>(zFar))},
	viewProjection {}, inverseViewProjection {}
{
	updateView();
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
	viewProjection = projection * glm::lookAt(origin, forward, up);
	inverseViewProjection = glm::inverse(viewProjection);
}

Direction3D
Camera::upFromForward(const Direction3D & forward)
{
	const auto right = glm::cross(forward, ::down);
	return glm::cross(forward, right);
}

std::array<GlobalPosition4D, 4>
Camera::extentsAtDist(const GlobalDistance dist) const
{
	const auto clampToSeaFloor = [this, dist](GlobalPosition3D target) -> GlobalPosition4D {
		target += position;
		if (target.z < -1500) {
			const auto diff = target - position;
			const auto limit = -1500 - position.z;
			return {position + (limit * diff) / diff.z, (limit * dist) / diff.z};
		}
		return {target, dist};
	};
	const auto depth = -(2.F * (static_cast<float>(dist - near)) * static_cast<float>(far))
					/ (static_cast<float>(dist) * (static_cast<float>(near - far)))
			- 1.F;
	static constexpr const std::array extents {-1.F, 1.F};
	static constexpr const auto cartesianExtents = extents * extents;
	return cartesianExtents * [&depth, this, &clampToSeaFloor](const auto & extent) {
		const glm::vec4 in {extent.first, extent.second, depth, 1.F};
		return clampToSeaFloor(perspective_divide(inverseViewProjection * in));
	};
}
