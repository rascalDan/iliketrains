#pragma once

#include "config/types.h"
#include "gfx/gl/vertexArrayObject.h"
#include <glArrays.h>
#include <glad/gl.h>
#include <ranges>
#include <span>
#include <stdTypeDefs.h>

class Vertex;

class MeshBase {
public:
	class Dimensions {
	public:
		using Extents1D = std::ranges::minmax_result<RelativeDistance>;
		explicit Dimensions(const std::span<const RelativePosition3D>);

		RelativePosition3D minExtent, maxExtent;
		RelativePosition3D centre;
		RelativeDistance size;

	private:
		Dimensions(const std::span<const RelativePosition3D>, const std::array<Extents1D, 3> &);
		static Extents1D extents(const std::span<const RelativePosition3D>, glm::length_t D);
	};

	void Draw() const;
	void DrawInstanced(GLuint vao, GLsizei count, GLuint base = 0) const;

	[[nodiscard]] const Dimensions &
	getDimensions() const
	{
		return dimensions;
	}

protected:
	MeshBase(GLsizei m_numIndices, GLenum mode, const std::vector<RelativePosition3D> &);

	glVertexArray m_vertexArrayObject;
	glBuffers<2> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
	Dimensions dimensions;
};

template<typename V> class MeshT : public MeshBase, public ConstTypeDefs<MeshT<V>> {
public:
	MeshT(const std::span<const V> vertices, const std::span<const unsigned int> indices, GLenum mode = GL_TRIANGLES) :
		MeshBase {static_cast<GLsizei>(indices.size()), mode,
				materializeRange(vertices | std::views::transform([](const auto & v) {
					return static_cast<RelativePosition3D>(v.pos);
				}))}
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
