#pragma once

#include <glm/glm.hpp>
#include <ray.hpp>

class Camera {
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);

	[[nodiscard]] glm::mat4 GetViewProjection() const;
	[[nodiscard]] Ray unProject(const glm::vec2 &) const;

	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;

private:
	glm::mat4 projection;
};
