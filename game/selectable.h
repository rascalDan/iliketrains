#pragma once

#include <glm/glm.hpp>
#include <special_members.hpp>

class Selectable {
public:
	Selectable() = default;
	virtual ~Selectable() = default;
	DEFAULT_MOVE_COPY(Selectable);

	[[nodiscard]] virtual bool intersectRay(const glm::vec3 &, const glm::vec3 &, glm::vec2 *, float *) const = 0;
};
