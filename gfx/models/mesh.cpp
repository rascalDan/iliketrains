#include "mesh.h"

MeshBase::MeshBase(GLsizei m_numIndices, GLenum mode) : m_numIndices {m_numIndices}, mode {mode} { }

void
MeshBase::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElements(mode, m_numIndices, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
}

void
MeshBase::DrawInstanced(GLuint vao, GLsizei count) const
{
	glBindVertexArray(vao);

	glDrawElementsInstanced(mode, m_numIndices, GL_UNSIGNED_INT, nullptr, count);

	glBindVertexArray(0);
}
