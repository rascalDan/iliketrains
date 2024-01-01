#pragma once

#include "config/types.h"
#include <glm/glm.hpp>
#include <span>

class Ray {
public:
#ifndef __cpp_aggregate_paren_init
	Ray(Position3D start, Direction3D direction) : start {start}, direction {direction} { }
#endif

	static Ray fromPoints(Position3D, Position3D);

	Position3D start;
	Direction3D direction;

	[[nodiscard]] float distanceToLine(const Position3D & a, const Position3D & b) const;
	[[nodiscard]] bool passesCloseToEdges(const std::span<const Position3D> positions, float distance) const;
};
