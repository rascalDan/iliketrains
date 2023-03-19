#pragma once

#include <glm/glm.hpp>

class Vertex {
public:
#ifndef __cpp_aggregate_paren_init
	constexpr Vertex(glm::vec3 pos, glm::vec2 texCoord, glm::vec3 normal, glm::vec4 colour = {}) :
		pos {std::move(pos)}, texCoord {std::move(texCoord)}, normal {std::move(normal)}, colour {std::move(colour)}
	{
	}
#endif

	bool operator==(const Vertex &) const = default;

	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 colour;
};
