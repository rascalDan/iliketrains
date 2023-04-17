#include "mesh.h"
#include "gfx/gl/vertexArrayObject.hpp"
#include "glArrays.h"
#include "vertex.hpp"
#include <cstddef>

Mesh::Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum m) :
	m_numIndices {static_cast<GLsizei>(indices.size())}, mode {m}
{
	VertexArrayObject {m_vertexArrayObject}
			.addAttribs<Vertex, &Vertex::pos, &Vertex::texCoord, &Vertex::normal, &Vertex::colour, &Vertex::material>(
					m_vertexArrayBuffers[0], vertices)
			.addIndices(m_vertexArrayBuffers[1], indices);
}

void
Mesh::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElements(mode, m_numIndices, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
}
