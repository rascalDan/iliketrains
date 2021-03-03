#ifndef LOCATION_H
#define LOCATION_H

#include <glm/glm.hpp>

class Location {
public:
	explicit Location(glm::vec3 pos = {}, glm::vec3 rot = {}) : pos {pos}, rot {rot} { }

	glm::vec3 pos;
	glm::vec3 rot;
};

#endif
