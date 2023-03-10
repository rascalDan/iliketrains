#pragma once

#include <GL/glew.h>
#include <glArrays.h>
#include <memory>
#include <span>
#include <stdTypeDefs.hpp>

class Vertex;

class Mesh : public ConstTypeDefs<Mesh> {
public:
	Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum = GL_TRIANGLES);

	void Draw() const;

private:
	glVertexArray m_vertexArrayObject;
	glBuffers<2> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
};
