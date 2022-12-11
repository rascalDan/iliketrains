#include "camera.h"
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <maths.h>
#include <ray.hpp>

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar) :
	position {pos}, forward {::north}, up {::up}, projection {glm::perspective(fov, aspect, zNear, zFar)}
{
}

glm::mat4
Camera::getViewProjection() const
{
	return projection * glm::lookAt(position, position + forward, up);
}

Ray
Camera::unProject(const glm::vec2 & mouse) const
{
	static constexpr const glm::vec4 screen {0, 0, 1, 1};
	return {position,
			glm::normalize(glm::unProject(mouse ^ 1, glm::lookAt(::origin, forward, up), projection, screen))};
}
