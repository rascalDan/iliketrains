#pragma once

#include <GL/glew.h>
#include <array>
#include <glm/common.hpp>

template<typename T> struct gl_traits;
template<> struct gl_traits<float> {
	static constexpr GLenum type {GL_FLOAT};
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
