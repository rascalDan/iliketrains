#include "mesh.h"
#include "glBuffers.h"
#include "glVertexArrays.h"
#include "vertex.hpp"
#include <cstddef>

#define offset_ptr(T, m) ((reinterpret_cast<char *>(1)) + offsetof(T, m) - 1)

Mesh::Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum m) :
	m_numIndices {static_cast<GLsizei>(indices.size())}, mode {m}
{
	glBindVertexArray(m_vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), vertices.data(),
			GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset_ptr(Vertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset_ptr(Vertex, texCoord));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset_ptr(Vertex, normal));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexArrayBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(indices[0]) * indices.size()), indices.data(),
			GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void
Mesh::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElementsBaseVertex(mode, m_numIndices, GL_UNSIGNED_INT, nullptr, 0);

	glBindVertexArray(0);
}
