#include "camera.h"
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
	const auto adjustToSeafloor = [this](glm::vec3 & target) {
		const auto vec = glm::normalize(target - position);
		constexpr glm::vec3 seafloor {0, 0, -1.5};
		float outdist;
		if (glm::intersectRayPlane(position, vec, seafloor, ::up, outdist)) {
			target = vec * outdist + position;
		}
	};
	const auto depth = -(2.f * (dist - near) * far) / (dist * (near - far)) - 1.f;
	static constexpr const std::array extents {-1.F, 1.F};
	std::array<glm::vec3, 4> out {};
	auto outitr = out.begin();
	for (auto x : extents) {
		for (auto y : extents) {
			const glm::vec4 in {x, y, depth, 1.f};

			const auto out = inverseViewProjection * in;
			*outitr = out / out.w;
			if (outitr->z < -1.5f) {
				adjustToSeafloor(*outitr);
			}
			outitr++;
		}
	}
	return out;
}
