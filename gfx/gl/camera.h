#pragma once

#include "config/types.h"
#include <glm/glm.hpp>
#include <maths.h>
#include <ray.h>

class Camera {
public:
	Camera(Position3D, Angle fov, Angle aspect, Distance zNear, Distance zFar);

	[[nodiscard]] glm::mat4
	getViewProjection() const
	{
		return viewProjection;
	}

	[[nodiscard]] Ray unProject(const ScreenRelCoord &) const;

	void
	setPosition(const Position3D & p)
	{
		position = p;
		updateView();
	}

	void
	setForward(const Direction3D & f)
	{
		setForward(f, upFromForward(f));
	}

	void
	setForward(const Direction3D & f, const Direction3D & u)
	{
		forward = f;
		up = u;
		updateView();
	}

	void
	setView(const Position3D & p, const Direction3D & f)
	{
		position = p;
		setForward(f);
	}

	void
	setView(const Position3D & p, const Direction3D & f, const Direction3D & u)
	{
		position = p;
		setView(f, u);
	}

	void
	lookAt(const Position3D & target)
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

	[[nodiscard]] static Direction3D upFromForward(const Direction3D & forward);

private:
	void updateView();

	Position3D position;
	Direction3D forward;
	Direction3D up;

	float near, far;
	glm::mat4 projection;
	glm::mat4 viewProjection, inverseViewProjection;
};
