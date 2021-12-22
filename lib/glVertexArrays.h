#ifndef GLVERTEXARRAYS_H
#define GLVERTEXARRAYS_H

#include <GL/glew.h>
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
	NO_MOVE(glVertexArrays);

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
using glVertexArray = glVertexArrays<1>;

#endif
