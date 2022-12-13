#include "camera.h"
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
	return {position, glm::normalize(glm::unProject(mouse ^ 1, unView, projection, screen))};
}

void
Camera::updateView()
{
	view = glm::lookAt(position, position + forward, up);
	unView = glm::lookAt(::origin, forward, up);
	viewProjection = projection * view;
	inverseViewProjection = glm::inverse(viewProjection);
}

glm::vec3
Camera::upFromForward(const glm::vec3 & forward)
{
	const auto right = glm::cross(forward, ::down);
	return glm::cross(forward, right);
}
