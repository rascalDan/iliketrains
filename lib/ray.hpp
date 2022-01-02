#pragma once

#include <glm/glm.hpp>

class Ray {
public:
	Ray(glm::vec3 start, glm::vec3 direction) : start {start}, direction {direction} { }

	glm::vec3 start;
	glm::vec3 direction;
};
