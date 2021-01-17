#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

class Vertex {
public:
	Vertex(glm::vec3 pos, glm::vec2 texCoord, glm::vec3 normal) :
		pos {std::move(pos)}, texCoord {std::move(texCoord)}, normal {std::move(normal)}
	{
	}

	glm::vec3 &
	GetPos()
	{
		return pos;
	}

	glm::vec2 &
	GetTexCoord()
	{
		return texCoord;
	}

	glm::vec3 &
	GetNormal()
	{
		return normal;
	}

private:
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

#endif
