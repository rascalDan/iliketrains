#pragma once

#include "collections.hpp"
#include "gl_traits.hpp"
#include <GL/glew.h>
#include <glm/common.hpp>

template<typename Vertex> class VertexArrayObject {
public:
	template<auto Vertex::*... attribs>
	static void
	configure(const GLuint arrayObject, const GLuint arrayBuffer, const GLuint indexBuffer,
			const SequentialCollection<Vertex> auto & vertices, const SequentialCollection<unsigned int> auto & indices)
	{
		glBindVertexArray(arrayObject);

		configure_attribs<attribs...>(arrayBuffer);
		data(vertices, arrayBuffer, GL_ARRAY_BUFFER);
		data(indices, indexBuffer, GL_ELEMENT_ARRAY_BUFFER);

		glBindVertexArray(0);
	}

	template<auto Vertex::*... attribs>
	static void
	configure(const GLuint arrayObject, const GLuint arrayBuffer, const SequentialCollection<Vertex> auto & vertices)
	{
		glBindVertexArray(arrayObject);

		configure_attribs<attribs...>(arrayBuffer);
		data(vertices, arrayBuffer, GL_ARRAY_BUFFER);

		glBindVertexArray(0);
	}

	template<auto Vertex::*... attribs>
	static void
	configure(const GLuint arrayObject, const GLuint arrayBuffer)
	{
		glBindVertexArray(arrayObject);

		configure_attribs<attribs...>(arrayBuffer);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex)), nullptr, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}

private:
	template<typename Data>
	static void
	data(const Data & data, const GLuint arrayBuffer, GLenum target)
	{
		using Value = typename Data::value_type;
		glBindBuffer(target, arrayBuffer);
		glBufferData(target, static_cast<GLsizeiptr>(sizeof(Value) * data.size()), data.data(), GL_STATIC_DRAW);
	}

	template<auto Vertex::*attrib>
	static void
	set_pointer(GLuint & vertexArrayId)
	{
		glEnableVertexAttribArray(vertexArrayId);
		using traits = gl_traits<std::decay_t<decltype(std::declval<Vertex>().*attrib)>>;
		glVertexAttribPointer(vertexArrayId++, traits::size, traits::type, GL_FALSE, sizeof(Vertex),
				&(static_cast<const Vertex *>(nullptr)->*attrib));
	}

	template<auto Vertex::*... attribs>
	static void
	configure_attribs(const GLuint arrayBuffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
		GLuint vertexArrayId {};
		(set_pointer<attribs>(vertexArrayId), ...);
	}
};
