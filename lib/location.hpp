#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Location {
public:
	explicit Location(glm::vec3 pos = {}, glm::vec3 rot = {}) : pos {pos}, rot {rot} { }

	glm::mat4 getTransform() const;

	glm::vec3 pos;
	glm::vec3 rot;
};
