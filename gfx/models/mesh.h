#pragma once

#include "config/types.h"
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

	auto
	minExtent() const
	{
		return min;
	}

	auto
	maxExtent() const
	{
		return max;
	}

protected:
	MeshBase(GLsizei m_numIndices, GLenum mode, std::pair<RelativePosition3D, RelativePosition3D> minmax);

	glVertexArray m_vertexArrayObject;
	glBuffers<2> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
	RelativePosition3D min, max;
};

template<typename V> class MeshT : public MeshBase, public ConstTypeDefs<MeshT<V>> {
public:
	MeshT(const std::span<const V> vertices, const std::span<const unsigned int> indices, GLenum mode = GL_TRIANGLES) :
		MeshBase {static_cast<GLsizei>(indices.size()), mode, extent(vertices)}
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

	static auto
	extent(const std::span<const V> vertices)
	{
		std::pair<decltype(V::pos), decltype(V::pos)> out {};
		for (glm::length_t D {}; D < 3; ++D) {
			const auto mm
					= std::minmax_element(vertices.begin(), vertices.end(), [D](const auto & va, const auto & vb) {
						  return va.pos[D] < vb.pos[D];
					  });
			out.first[D] = mm.first->pos[D];
			out.second[D] = mm.second->pos[D];
		}
		return out;
	}
};

using Mesh = MeshT<Vertex>;
