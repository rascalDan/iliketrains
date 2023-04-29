#pragma once

#include <glm/glm.hpp>
#include <span>

class Ray {
public:
	Ray(glm::vec3 start, glm::vec3 direction) : start {start}, direction {direction} { }

	static Ray fromPoints(glm::vec3, glm::vec3);

	glm::vec3 start;
	glm::vec3 direction;

	float distanceToLine(const glm::vec3 & a, const glm::vec3 & b) const;
	bool passesCloseToEdges(const std::span<const glm::vec3> positions, float distance) const;
};
