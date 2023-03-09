#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Mutation {
	using Matrix = glm::mat4;

	Matrix getMatrix() const;
	Matrix getDeformationMatrix() const;
	Matrix getLocationMatrix() const;

	float relativeLevelOfDetail() const;

	glm::vec3 position {};
	glm::vec3 rotation {};
	glm::vec3 scale {1};
};
