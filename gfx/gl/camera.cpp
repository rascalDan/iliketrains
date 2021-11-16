#include "camera.h"
#include <glm/gtx/transform.hpp>
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
