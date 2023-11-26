#pragma once

#include "config/types.h"
#include <glm/glm.hpp>
#include <special_members.h>

class Ray;

class Selectable {
public:
	Selectable() = default;
	virtual ~Selectable() = default;
	DEFAULT_MOVE_COPY(Selectable);

	[[nodiscard]] virtual bool intersectRay(const Ray &, Position2D *, float *) const = 0;
};
