#pragma once

#include "aabb.h"
#include "config/types.h"
#include <array>
#include <glm/mat4x4.hpp>
#include <span>

class Frustum {
public:
	Frustum(const GlobalPosition3D & pos, const glm::mat4 & view, const glm::mat4 & projection);

	[[nodiscard]] auto &
	getFrustumPlanes() const
	{
		return planes;
	}

	[[nodiscard]] auto &
	getViewProjection() const
	{
		return viewProjection;
	}

	[[nodiscard]] auto
	getPosition() const
	{
		return position;
	}

	void updateView(const glm::mat4 & view);

	using BoundingBox = AxisAlignedBoundingBox<GlobalDistance>;
	[[nodiscard]] bool contains(const BoundingBox &) const;
	[[nodiscard]] bool contains(GlobalPosition3D, RelativeDistance size = 0) const;

protected:
	static constexpr size_t FACES = 5;
	void updateCache();
	[[nodiscard]] bool contains(std::span<const RelativePosition4D>, RelativeDistance) const;

	GlobalPosition3D position;
	glm::mat4 view, projection;
	glm::mat4 viewProjection, inverseViewProjection;
	std::array<glm::vec4, FACES> planes;
};
