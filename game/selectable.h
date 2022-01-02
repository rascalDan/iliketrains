#pragma once

#include <glm/glm.hpp>
#include <special_members.hpp>

class Ray;

class Selectable {
public:
	Selectable() = default;
	virtual ~Selectable() = default;
	DEFAULT_MOVE_COPY(Selectable);

	[[nodiscard]] virtual bool intersectRay(const Ray &, glm::vec2 *, float *) const = 0;
};
