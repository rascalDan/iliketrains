#pragma once

#include "collections.h"
#include "gl_traits.h"
#include "special_members.h"
#include <glad/gl.h>

class VertexArrayObject {
public:
	template<typename T> [[nodiscard]] VertexArrayObject(const T & arrayObject)
	{
		glBindVertexArray(arrayObject);
	}

	~VertexArrayObject()
	{
		glBindVertexArray(0);
	}

	NO_MOVE(VertexArrayObject);
	NO_COPY(VertexArrayObject);

	template<typename m, typename T> struct MP {
		constexpr MP(m T::* p) : P {p} { }

		constexpr
		operator void *() const
		{
			return &(static_cast<T *>(nullptr)->*P);
		}

		m T::* P;
		using value_type = m;
	};

	template<typename m, typename T> MP(m T::*) -> MP<m, T>;

	template<typename VertexT, MP... attribs>
	VertexArrayObject &
	addAttribs(const GLuint arrayBuffer, const SequentialCollection<VertexT> auto & vertices, const GLuint divisor = 0)
	{
		addAttribs<VertexT, attribs...>(arrayBuffer, divisor);
		data(vertices, arrayBuffer, GL_ARRAY_BUFFER);
		return *this;
	}

	template<typename VertexT, MP... attribs>
	VertexArrayObject &
	addAttribs(const GLuint arrayBuffer, const GLuint divisor = 0)
	{
		configure_attribs<VertexT, attribs...>(arrayBuffer, divisor);
		return *this;
	}

	// Customisation point
	template<typename VertexT> VertexArrayObject & addAttribsFor(const GLuint arrayBuffer, const GLuint divisor = 0);

	template<typename Indices>
	VertexArrayObject &
	addIndices(const GLuint arrayBuffer, const Indices & indices)
	{
		data(indices, arrayBuffer, GL_ELEMENT_ARRAY_BUFFER);
		return *this;
	}

	VertexArrayObject &
	addIndices(const GLuint arrayBuffer)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrayBuffer);
		return *this;
	}

	VertexArrayObject &
	data(const GLuint arrayBuffer, GLenum target)
	{
		glBindBuffer(target, arrayBuffer);
		return *this;
	}

	template<typename Data>
	static void
	data(const Data & data, const GLuint arrayBuffer, GLenum target)
	{
		using Value = typename Data::value_type;
		glBindBuffer(target, arrayBuffer);
		glBufferData(target, static_cast<GLsizeiptr>(sizeof(Value) * data.size()), data.data(), GL_STATIC_DRAW);
	}

private:
	template<typename VertexT, typename T>
	static auto
	set_pointer(const GLuint vertexArrayId, const void * ptr, const GLuint divisor)
	{
		using traits = gl_traits<T>;
		const auto usedAttribs
				= traits::vertexAttribFunc(vertexArrayId, traits::size, traits::type, sizeof(VertexT), ptr);
		for (GLuint i {}; i < usedAttribs; i++) {
			glEnableVertexAttribArray(vertexArrayId + i);
			glVertexAttribDivisor(vertexArrayId + i, divisor);
		}
		return usedAttribs;
	}

	template<typename VertexT, MP attrib>
	static auto
	set_pointer(const GLuint vertexArrayId, const GLuint divisor)
	{
		return set_pointer<VertexT, typename decltype(attrib)::value_type>(vertexArrayId, attrib, divisor);
	}

	template<typename VertexT, MP... attribs>
	void
	configure_attribs(const GLuint arrayBuffer, const GLuint divisor)
	{
		glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
		if constexpr (sizeof...(attribs) == 0) {
			vertexArrayId += set_pointer<VertexT, VertexT>(vertexArrayId, nullptr, divisor);
		}
		else {
			((vertexArrayId += set_pointer<VertexT, attribs>(vertexArrayId, divisor)), ...);
		}
	}

	GLuint vertexArrayId {};
};
