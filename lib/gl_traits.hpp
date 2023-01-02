#pragma once

#include <GL/glew.h>
#include <array>
#include <glm/common.hpp>
#include <glm/fwd.hpp>

template<typename T> struct gl_traits;
template<> struct gl_traits<glm::f32> {
	static constexpr GLenum type {GL_FLOAT};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::f64> {
	static constexpr GLenum type {GL_DOUBLE};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::int8> {
	static constexpr GLenum type {GL_BYTE};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::int16> {
	static constexpr GLenum type {GL_SHORT};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::int32> {
	static constexpr GLenum type {GL_INT};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::uint8> {
	static constexpr GLenum type {GL_UNSIGNED_BYTE};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::uint16> {
	static constexpr GLenum type {GL_UNSIGNED_SHORT};
	static constexpr GLint size {1};
};
template<> struct gl_traits<glm::uint32> {
	static constexpr GLenum type {GL_UNSIGNED_INT};
	static constexpr GLint size {1};
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
};
