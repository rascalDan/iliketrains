#include "mesh.h"
#include "gfx/gl/vertexArrayObject.hpp"
#include "glArrays.h"
#include "vertex.hpp"
#include <cstddef>

Mesh::Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum m) :
	m_numIndices {static_cast<GLsizei>(indices.size())}, mode {m}
{
	VertexArrayObject::data(vertices, m_vertexArrayBuffers[0], GL_ARRAY_BUFFER);
	VertexArrayObject::data(indices, m_vertexArrayBuffers[1], GL_ARRAY_BUFFER);
	configureVAO(m_vertexArrayObject);
}

VertexArrayObject &
Mesh::configureVAO(VertexArrayObject && vao) const
{
	return vao
			.addAttribs<Vertex, &Vertex::pos, &Vertex::texCoord, &Vertex::normal, &Vertex::colour, &Vertex::material>(
					m_vertexArrayBuffers[0])
			.addIndices(m_vertexArrayBuffers[1]);
}

void
Mesh::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElements(mode, m_numIndices, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
}

void
Mesh::DrawInstanced(GLuint vao, GLsizei count) const
{
	glBindVertexArray(vao);

	glDrawElementsInstanced(mode, m_numIndices, GL_UNSIGNED_INT, nullptr, count);

	glBindVertexArray(0);
}
