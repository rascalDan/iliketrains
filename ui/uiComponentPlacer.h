#pragma once

#include <glm/glm.hpp>

class UIComponentPlacer {
public:
	UIComponentPlacer(glm::vec2 padding, float spacing, glm::length_t axis = 0);

	glm::vec2 next(glm::vec2 size);
	glm::vec2 getLimit() const;

private:
	const glm::vec2 padding;
	const float spacing;
	const glm::length_t axis;

	float current {};
	float max {};
};
