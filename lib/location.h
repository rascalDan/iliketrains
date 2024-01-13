#pragma once

#include "config/types.h"
#include <glm/mat4x4.hpp>

class Location {
public:
#ifndef __cpp_aggregate_paren_init
	explicit Location(GlobalPosition3D pos = {}, Rotation3D rot = {}) : pos {pos}, rot {rot} { }
#endif

	[[nodiscard]] glm::mat3 getRotationTransform() const;

	GlobalPosition3D pos;
	Rotation3D rot;
};
