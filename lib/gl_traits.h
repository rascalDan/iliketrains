#pragma once

#include <array>
#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>

template<typename T> struct gl_traits;

struct gl_traits_base {
	static constexpr GLint size {1};
};

struct gl_traits_float : public gl_traits_base {
	static constexpr auto vertexAttribFunc {
			[](GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer) -> GLuint {
				glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
				return 1;
			}};
};

struct gl_traits_longfloat : public gl_traits_base {
	static constexpr auto vertexAttribFunc {
			[](GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer) -> GLuint {
				glVertexAttribLPointer(index, size, type, stride, pointer);
				return 1;
			}};
};

struct gl_traits_integer : public gl_traits_base {
	static constexpr auto vertexAttribFunc {
			[](GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer) -> GLuint {
				glVertexAttribIPointer(index, size, type, stride, pointer);
				return 1;
			}};
};

template<> struct gl_traits<glm::f32> : public gl_traits_float {
	static constexpr GLenum type {GL_FLOAT};
};

template<> struct gl_traits<glm::f64> : public gl_traits_longfloat {
	static constexpr GLenum type {GL_DOUBLE};
};

template<> struct gl_traits<glm::int8> : public gl_traits_integer {
	static constexpr GLenum type {GL_BYTE};
};

template<> struct gl_traits<glm::int16> : public gl_traits_integer {
	static constexpr GLenum type {GL_SHORT};
};

template<> struct gl_traits<glm::int32> : public gl_traits_integer {
	static constexpr GLenum type {GL_INT};
};

template<> struct gl_traits<glm::uint8> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_BYTE};
};

template<> struct gl_traits<glm::uint16> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_SHORT};
};

template<> struct gl_traits<glm::uint32> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_INT};
};

template<typename T, std::size_t S> struct gl_traits<std::array<T, S>> : public gl_traits<T> {
	static constexpr GLint size {S * gl_traits<T>::size};
};

template<glm::length_t L, typename T, glm::qualifier Q> struct gl_traits<glm::vec<L, T, Q>> : public gl_traits<T> {
	static constexpr GLint size {L};
};

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct gl_traits<glm::mat<C, R, T, Q>> : public gl_traits<T> {
	static constexpr GLint size {C * R};
	static constexpr auto vertexAttribFunc {
			[](GLuint index, GLint, GLenum type, GLsizei stride, const void * pointer) -> GLuint {
				const auto base = static_cast<const T *>(pointer);
				for (GLuint r = 0; r < R; r++) {
					glVertexAttribPointer(index + r, C, type, GL_FALSE, stride, base + (r * C));
				}
				return R;
			}};
};
