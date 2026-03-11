#pragma once

#include "config/types.h"
#include "gfx/gl/glBuffer.h"
#include "gfx/gl/gldebug.h"
#include <gfx/gl/glVertexArray.h>
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
		explicit Dimensions(std::span<const RelativePosition3D>);

		RelativePosition3D minExtent, maxExtent;
		RelativePosition3D centre;
		RelativeDistance size;

	private:
		Dimensions(std::span<const RelativePosition3D>, const std::array<Extents1D, 3> &);
		static Extents1D extents(std::span<const RelativePosition3D>, glm::length_t);
	};

	void draw() const;
	void drawInstanced(GLuint vao, GLsizei count, GLuint base = 0) const;

	[[nodiscard]] const Dimensions &
	getDimensions() const
	{
		return dimensions;
	}

protected:
	MeshBase(GLsizei numIndices, GLenum mode, const std::vector<RelativePosition3D> &, GLsizei vertexStride);

	std::shared_ptr<glVertexArray> vertexArrayObject;
	glBuffers<2> vertexArrayBuffers;
	GLsizei vertexStride;
	GLsizei numIndices;
	GLenum mode;
	Dimensions dimensions;
};

template<typename V> class MeshT : public MeshBase, public ConstTypeDefs<MeshT<V>> {
public:
	MeshT(const std::span<const V> vertices, const std::span<const unsigned int> indices, GLenum mode = GL_TRIANGLES) :
		MeshBase {static_cast<GLsizei>(indices.size()), mode,
				materializeRange(vertices | std::views::transform([](const auto & vertex) {
					return static_cast<RelativePosition3D>(vertex.pos);
				})),
				sizeof(V)}
	{
		glDebugScope _ {0};
		vertexArrayBuffers[0].storage(vertices, 0);
		vertexArrayBuffers[1].storage(indices, 0);
		if (!(vertexArrayObject = commonVertexArrayObject.lock())) {
			commonVertexArrayObject = vertexArrayObject = std::make_shared<glVertexArray>();
			configureVAO(*vertexArrayObject, 0);
		}
	}

	auto
	configureVAO(glVertexArray & vao, GLuint divisor) const
	{
		glDebugScope _ {0};
		return vao.configure().addAttribsFor<V>(divisor);
	}

protected:
	static std::weak_ptr<glVertexArray> commonVertexArrayObject;
};

template<typename T> std::weak_ptr<glVertexArray> MeshT<T>::commonVertexArrayObject;

using Mesh = MeshT<Vertex>;
