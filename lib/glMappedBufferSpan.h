#pragma once

#include "special_members.h"
#include <cstddef>
#include <glad/gl.h>
#include <span>
#include <utility>

template<typename T> class glMappedBufferSpan : public std::span<T> {
public:
	glMappedBufferSpan(GLuint buffer, size_t count, GLenum access, bool reinit) :
		std::span<T> {[&]() {
						  if (reinit) {
							  glNamedBufferData(
									  buffer, static_cast<GLsizeiptr>(sizeof(T) * count), nullptr, GL_DYNAMIC_DRAW);
						  }
						  return static_cast<T *>(glMapNamedBuffer(buffer, access));
					  }(),
				count},
		buffer {buffer}
	{
	}

	~glMappedBufferSpan()
	{
		if (buffer) {
			glUnmapNamedBuffer(buffer);
		}
	}

	glMappedBufferSpan(glMappedBufferSpan && other) noexcept :
		std::span<T> {other}, buffer {std::exchange(other.buffer, 0)}
	{
	}

	glMappedBufferSpan &
	operator=(glMappedBufferSpan && other) noexcept
	{
		std::span<T>::span = other;
		if (buffer) {
			glUnmapBuffer(buffer);
		}
		buffer = std::exchange(other.buffer, 0);
		return *this;
	}

	NO_COPY(glMappedBufferSpan);

private:
	GLuint buffer;
};
