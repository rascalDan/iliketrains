#pragma once

#include <glm/glm.hpp>
#include <maths.h>
#include <ray.hpp>

class Camera {
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);

	[[nodiscard]] glm::mat4
	getViewProjection() const
	{
		return viewProjection;
	}
	[[nodiscard]] Ray unProject(const glm::vec2 &) const;

	void
	setPosition(const glm::vec3 & p)
	{
		position = p;
		updateView();
	}
	void
	setForward(const glm::vec3 & f)
	{
		setForward(f, upFromForward(f));
	}
	void
	setForward(const glm::vec3 & f, const glm::vec3 & u)
	{
		forward = f;
		up = u;
		updateView();
	}
	void
	setView(const glm::vec3 & p, const glm::vec3 & f)
	{
		position = p;
		setForward(f);
	}
	void
	setView(const glm::vec3 & p, const glm::vec3 & f, const glm::vec3 & u)
	{
		position = p;
		setView(f, u);
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

	[[nodiscard]] std::array<glm::vec4, 4> extentsAtDist(float) const;

	[[nodiscard]] static glm::vec3 upFromForward(const glm::vec3 & forward);

private:
	void updateView();

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;

	float fov, aspect, near, far;
	glm::mat4 projection;
	glm::mat4 viewProjection, inverseViewProjection;
};
