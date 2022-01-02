#pragma once

#include <glm/glm.hpp>

class Location {
public:
	explicit Location(glm::vec3 pos = {}, glm::vec3 rot = {}) : pos {pos}, rot {rot} { }

	glm::vec3 pos;
	glm::vec3 rot;
};
