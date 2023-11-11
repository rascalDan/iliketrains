#pragma once

#include "config/types.h"
#include <glm/mat4x4.hpp>

class Location {
public:
#ifndef __cpp_aggregate_paren_init
	explicit Location(Position3D pos = {}, Rotation3D rot = {}) : pos {pos}, rot {rot} { }
#endif

	[[nodiscard]] glm::mat4 getTransform() const;
	[[nodiscard]] glm::mat4 getRotationTransform() const;

	Position3D pos;
	Rotation3D rot;
};
