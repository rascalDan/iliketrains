#pragma once

#include <glm/glm.hpp>

class Vertex {
public:
	constexpr Vertex(glm::vec3 pos, glm::vec2 texCoord, glm::vec3 normal, glm::vec4 colour = {}) :
		pos {std::move(pos)}, texCoord {std::move(texCoord)}, normal {std::move(normal)}, colour {std::move(colour)}
	{
	}

	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 colour;
};
