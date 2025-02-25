#pragma once

#include "special_members.h"
#include <cstddef>
#include <glad/gl.h>
#include <utility>

template<typename T> class glMappedBufferWriter {
public:
	using difference_type = std::ptrdiff_t;

	glMappedBufferWriter(GLenum target, GLuint buffer, size_t count, GLenum usage = GL_STATIC_DRAW) :
		target {target}, data {[&]() {
			glBindBuffer(target, buffer);
			glBufferData(target, static_cast<GLsizeiptr>(sizeof(T) * count), nullptr, usage);
			return static_cast<T *>(glMapBuffer(target, GL_WRITE_ONLY));
		}()}
	{
	}

	~glMappedBufferWriter()
	{
		if (target) {
			glUnmapBuffer(target);
		}
	}

	glMappedBufferWriter(glMappedBufferWriter && other) noexcept :
		target {std::exchange(other.target, 0)}, data {std::exchange(other.data, nullptr)}
	{
	}

	glMappedBufferWriter &
	operator=(glMappedBufferWriter && other) noexcept
	{
		if (target) {
			glUnmapBuffer(target);
		}
		target = std::exchange(other.target, 0);
		data = std::exchange(other.data, nullptr);
		return *this;
	}

	NO_COPY(glMappedBufferWriter);

	glMappedBufferWriter &
	operator++()
	{
		data++;
		return *this;
	}

	glMappedBufferWriter &
	operator++(int)
	{
		glMappedBufferWriter rtn {data};
		data++;
		return rtn;
	}

	T &
	operator*()
	{
		return *data;
	}

	T *
	operator->() const
	{
		return data;
	}

private:
	explicit glMappedBufferWriter(T * data) : target {0}, data {data} { }

	GLenum target;
	T * data;
};
