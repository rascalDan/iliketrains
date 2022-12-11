#pragma once

#include <glm/glm.hpp>
#include <maths.h>
#include <ray.hpp>

class Camera {
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);

	[[nodiscard]] glm::mat4 getViewProjection() const;
	[[nodiscard]] Ray unProject(const glm::vec2 &) const;

	void
	setPosition(const glm::vec3 & p)
	{
		position = p;
	}
	void
	setForward(const glm::vec3 & f)
	{
		forward = f;
	}
	void
	setView(const glm::vec3 & p, const glm::vec3 & f, const glm::vec3 & u = ::up)
	{
		position = p;
		forward = f;
		up = u;
	}
	void
	lookAt(const glm::vec3 & target)
	{
		setForward(glm::normalize(target - position));
	}
	[[nodiscard]] auto
	getForward() const
	{
		return forward;
	}
	[[nodiscard]] auto
	getPosition() const
	{
		return position;
	}

private:
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;

	glm::mat4 projection;
};
