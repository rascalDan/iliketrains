#include "camera.h"
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <maths.h>

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar) :
	pos {pos}, forward {::north}, up {::up}, projection {glm::perspective(fov, aspect, zNear, zFar)}
{
}

glm::mat4
Camera::GetViewProjection() const
{
	return projection * glm::lookAt(pos, pos + forward, up);
}

glm::vec3
Camera::unProject(const glm::vec2 & mouse) const
{
	static constexpr const glm::vec4 screen {0, 0, 1, 1};
	return glm::normalize(glm::unProject(mouse ^ 1, glm::lookAt(::origin, forward, up), projection, screen));
}
