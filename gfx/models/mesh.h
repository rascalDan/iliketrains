#ifndef MESH_INCLUDED_H
#define MESH_INCLUDED_H

#include <GL/glew.h>
#include <glArrays.h>
#include <memory>
#include <span>

class Vertex;

class Mesh {
public:
	Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum = GL_TRIANGLES);

	void Draw() const;

private:
	glVertexArray m_vertexArrayObject;
	glBuffers<2> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
};
using MeshPtr = std::shared_ptr<const Mesh>;

#endif
