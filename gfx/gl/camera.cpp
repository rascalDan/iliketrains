#include "camera.h"
#include <glm/gtx/transform.hpp>

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar) :
	pos {pos}, forward {0.0F, 0.0F, 1.0F}, up {0.0F, 1.0F, 0.0F}, projection {
																		  glm::perspective(fov, aspect, zNear, zFar)}
{
}

glm::mat4
Camera::GetViewProjection() const
{
	return projection * glm::lookAt(pos, pos + forward, up);
}

void
Camera::MoveForward(float amt)
{
	pos += forward * amt;
}

void
Camera::SlideForward(float amt)
{
	pos += forward * amt * glm::vec3 {1, 0, 1};
}

void
Camera::MoveRight(float amt)
{
	pos += glm::cross(up, forward) * amt;
}

void
Camera::Pitch(float angle)
{
	const auto right = glm::normalize(glm::cross(up, forward));

	forward = glm::vec3(glm::normalize(glm::rotate(angle, right) * glm::vec4(forward, 0.0)));
	up = glm::normalize(glm::cross(forward, right));
}

void
Camera::RotateY(float angle)
{
	static constexpr glm::vec3 UP {0.0F, 1.0F, 0.0F};

	const auto rotation = glm::rotate(angle, UP);

	forward = glm::vec3(glm::normalize(rotation * glm::vec4(forward, 0.0)));
	up = glm::vec3(glm::normalize(rotation * glm::vec4(up, 0.0)));
}
