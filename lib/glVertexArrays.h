#ifndef GLVERTEXARRAYS_H
#define GLVERTEXARRAYS_H

#include <GL/glew.h>
#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <special_members.hpp>

class glVertexArraysBase {
protected:
	static void gen(GLsizei, GLuint *);
	static void del(GLsizei, const GLuint *);
};

template<size_t N> class glVertexArrays : glVertexArraysBase {
public:
	glVertexArrays()
	{
		gen(N, ids.data());
	}

	~glVertexArrays()
	{
		del(N, ids.data());
	}

	NO_COPY(glVertexArrays);
	CUSTOM_MOVE(glVertexArrays);

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	operator GLuint() const
	{
		static_assert(N == 1, "Implicit cast only if N == 1");
		return ids.front();
	}

	auto
	operator[](size_t n) const
	{
		return ids[n];
	}

private:
	std::array<GLuint, N> ids {};
};

template<size_t N> glVertexArrays<N>::glVertexArrays(glVertexArrays<N> && src) noexcept : ids {src.ids}
{
	std::fill(src.ids.begin(), src.ids.end(), -1);
}

template<size_t N>
glVertexArrays<N> &
glVertexArrays<N>::operator=(glVertexArrays<N> && src) noexcept
{
	ids = src.ids;
	std::fill(src.ids.begin(), src.ids.end(), -1);
	return *this;
}

using glVertexArray = glVertexArrays<1>;

#endif
