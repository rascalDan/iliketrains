#pragma once

#include <glm/glm.hpp>

class Vertex {
public:
	Vertex(glm::vec3 pos, glm::vec2 texCoord, glm::vec3 normal) :
		pos {std::move(pos)}, texCoord {std::move(texCoord)}, normal {std::move(normal)}
	{
	}

	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
};
