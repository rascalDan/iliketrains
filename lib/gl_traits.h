#pragma once

#include <array>
#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <span>

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
	static constexpr auto glUniformFunc {&glUniform1f};
	static constexpr std::array glUniformvFunc {&glUniform1fv, &glUniform2fv, &glUniform3fv, &glUniform4fv};
	static constexpr std::array glUniformmFunc {&glUniformMatrix2fv, &glUniformMatrix3fv, &glUniformMatrix4fv};
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
	static constexpr auto glUniformFunc {&glUniform1i};
	static constexpr std::array glUniformvFunc {&glUniform1iv, &glUniform2iv, &glUniform3iv, &glUniform4iv};
};

template<> struct gl_traits<glm::uint8> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_BYTE};
};

template<> struct gl_traits<glm::uint16> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_SHORT};
};

template<> struct gl_traits<glm::uint32> : public gl_traits_integer {
	static constexpr GLenum type {GL_UNSIGNED_INT};
	static constexpr auto glUniformFunc {&glUniform1ui};
	static constexpr std::array<decltype(&glUniform1uiv), 5> glUniformvFunc {
			&glUniform1uiv, &glUniform2uiv, &glUniform3uiv, &glUniform4uiv};
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

template<typename T>
void
glUniform(GLint location, T v)
{
	static_assert(
			requires { gl_traits<T>::glUniformFunc; }, "Has glUnform1T");
	(*gl_traits<T>::glUniformFunc)(location, v);
}

template<glm::length_t L, typename T, glm::qualifier Q>
void
glUniform(GLint location, const glm::vec<L, T, Q> & v)
{
	static_assert(
			requires { gl_traits<T>::glUniformFunc; }, "Has glUnformNTv");
	(*gl_traits<T>::glUniformvFunc[L - 1])(location, 1, glm::value_ptr(v));
}

template<glm::length_t L, typename T, glm::qualifier Q>
void
glUniform(GLint location, std::span<const glm::vec<L, T, Q>> v)
{
	static_assert(
			requires { gl_traits<T>::glUniformvFunc; }, "Has glUnformNTv");
	(*gl_traits<T>::glUniformvFunc[L - 1])(location, static_cast<GLsizei>(v.size()), glm::value_ptr(v.front()));
}

template<glm::length_t L, typename T, glm::qualifier Q>
void
glUniform(GLint location, const glm::mat<L, L, T, Q> & v)
{
	static_assert(
			requires { gl_traits<T>::glUniformmFunc; }, "Has glUnformMatrixNTv");
	(*gl_traits<T>::glUniformmFunc[L - 2])(location, 1, GL_FALSE, glm::value_ptr(v));
}

template<glm::length_t L, typename T, glm::qualifier Q>
void
glUniform(GLint location, std::span<const glm::mat<L, L, T, Q>> v)
{
	static_assert(
			requires { gl_traits<T>::glUniformmFunc; }, "Has glUnformMatrixNTv");
	(*gl_traits<T>::glUniformmFunc[L - 2])(
			location, static_cast<GLsizei>(v.size()), GL_FALSE, glm::value_ptr(v.front()));
}
