#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include <GL/glew.h>
#include <array>
#include <cstddef>
#include <special_members.hpp>

class glBuffersBase {
protected:
	static void gen(GLsizei, GLuint *);
	static void del(GLsizei, const GLuint *);
};

template<size_t N> class glBuffers : glBuffersBase {
public:
	glBuffers()
	{
		gen(N, ids.data());
	}

	~glBuffers()
	{
		del(N, ids.data());
	}

	NO_COPY(glBuffers);
	NO_MOVE(glBuffers);

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
using glBuffer = glBuffers<1>;

#endif
