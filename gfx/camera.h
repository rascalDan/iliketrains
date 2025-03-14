#pragma once

#include "config/types.h"
#include "frustum.h"
#include <glm/glm.hpp>
#include <maths.h>
#include <ray.h>

class Camera : public Frustum {
public:
	Camera(GlobalPosition3D position, Angle fov, Angle aspect, GlobalDistance near, GlobalDistance far);

	[[nodiscard]] Ray<GlobalPosition3D> unProject(const ScreenRelCoord &) const;

	void setAspect(Angle aspect);

	void
	setPosition(const GlobalPosition3D & p)
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
	setView(const GlobalPosition3D & p, const Direction3D & f)
	{
		position = p;
		setForward(f);
	}

	void
	setView(const GlobalPosition3D & p, const Direction3D & f, const Direction3D & u)
	{
		position = p;
		setView(f, u);
	}

	void
	lookAt(const GlobalPosition3D & target)
	{
		setForward(glm::normalize(RelativePosition3D(target - position)));
	}

	[[nodiscard]] auto
	getForward() const
	{
		return forward;
	}

	[[nodiscard]] std::array<GlobalPosition4D, 4> extentsAtDist(GlobalDistance) const;

	[[nodiscard]] static Direction3D upFromForward(const Direction3D & forward);

private:
	Camera(GlobalPosition3D position, Angle fov, Angle aspect, GlobalDistance near, GlobalDistance far,
			const glm::mat4 & view, const glm::mat4 & projection);
	void updateView();

	Angle fov, aspect;
	Direction3D forward;
	Direction3D up;
	GlobalDistance near, far;
};
