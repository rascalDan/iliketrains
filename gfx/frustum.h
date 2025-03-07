#pragma once

#include "config/types.h"
#include <array>
#include <glm/mat4x4.hpp>

class AxisAlignedBoundingBox;

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

	[[nodiscard]] bool contains(const AxisAlignedBoundingBox &) const;

protected:
	static constexpr size_t FACES = 6;
	void updateCache();

	GlobalPosition3D position;
	glm::mat4 view, projection;
	glm::mat4 viewProjection, inverseViewProjection;
	std::array<glm::vec4, FACES> planes;
};
