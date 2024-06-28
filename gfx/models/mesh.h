#pragma once

#include "gfx/gl/vertexArrayObject.h"
#include <glArrays.h>
#include <glad/gl.h>
#include <span>
#include <stdTypeDefs.h>

class Vertex;

class MeshBase {
public:
	void Draw() const;
	void DrawInstanced(GLuint vao, GLsizei count, GLuint base = 0) const;

protected:
	MeshBase(GLsizei m_numIndices, GLenum mode);

	glVertexArray m_vertexArrayObject;
	glBuffers<2> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
};

template<typename V> class MeshT : public MeshBase, public ConstTypeDefs<MeshT<V>> {
public:
	MeshT(const std::span<const V> vertices, const std::span<const unsigned int> indices, GLenum mode = GL_TRIANGLES) :
		MeshBase {static_cast<GLsizei>(indices.size()), mode}
	{
		VertexArrayObject::data(vertices, m_vertexArrayBuffers[0], GL_ARRAY_BUFFER);
		VertexArrayObject::data(indices, m_vertexArrayBuffers[1], GL_ARRAY_BUFFER);
		configureVAO(m_vertexArrayObject);
	}

	VertexArrayObject &
	configureVAO(VertexArrayObject && vao) const
	{
		return vao.addAttribsFor<V>(m_vertexArrayBuffers[0]).addIndices(m_vertexArrayBuffers[1]);
	}
};

using Mesh = MeshT<Vertex>;
