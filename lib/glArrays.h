#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <glad/gl.h>
#include <special_members.h>

template<size_t N> class glArraysBase {
	static_assert(N > 0);

public:
	~glArraysBase() = default;
	NO_COPY(glArraysBase);
	CUSTOM_MOVE(glArraysBase);

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	inline
	operator GLuint() const
	{
		static_assert(N == 1, "Implicit cast only if N == 1");
		return ids.front();
	}

	inline auto
	operator[](size_t n) const
	{
		return ids[n];
	}

	constexpr static auto size {N};

protected:
	glArraysBase() noexcept = default;
	std::array<GLuint, N> ids {};
};

template<size_t N> inline glArraysBase<N>::glArraysBase(glArraysBase<N> && src) noexcept : ids {src.ids}
{
	std::fill(src.ids.begin(), src.ids.end(), 0);
}

template<size_t N>
inline glArraysBase<N> &
glArraysBase<N>::operator=(glArraysBase<N> && src) noexcept
{
	ids = src.ids;
	std::fill(src.ids.begin(), src.ids.end(), 0);
	return *this;
}

template<size_t N, auto Gen, auto Del> class glArrays : public glArraysBase<N> {
public:
	using glArraysBase<N>::glArraysBase;
	using glArraysBase<N>::operator=;

	DEFAULT_MOVE_COPY(glArrays);

	inline glArrays() noexcept
	{
		(*Gen)(N, this->ids.data());
	}

	inline ~glArrays() noexcept
	{
		if (this->ids.front()) {
			(*Del)(N, this->ids.data());
		}
	}
};

template<size_t N> using glVertexArrays = glArrays<N, &glGenVertexArrays, &glDeleteVertexArrays>;
using glVertexArray = glVertexArrays<1>;
template<size_t N> using glBuffers = glArrays<N, &glGenBuffers, &glDeleteBuffers>;
using glBuffer = glBuffers<1>;
template<size_t N> using glTextures = glArrays<N, &glGenTextures, &glDeleteTextures>;
using glTexture = glTextures<1>;
template<size_t N> using glFrameBuffers = glArrays<N, &glGenFramebuffers, &glDeleteFramebuffers>;
using glFrameBuffer = glFrameBuffers<1>;
template<size_t N> using glRenderBuffers = glArrays<N, &glGenRenderbuffers, &glDeleteRenderbuffers>;
using glRenderBuffer = glRenderBuffers<1>;
