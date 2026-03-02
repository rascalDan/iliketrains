#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <glad/gl.h>
#include <special_members.h>
#include <utility>

namespace Detail {
	// NOLINTNEXTLINE(readability-identifier-naming)
	class glNamed;

	template<typename Named>
	concept IsglNamed = sizeof(Named) == sizeof(GLuint) && std::is_base_of_v<Detail::glNamed, Named>;
}

template<Detail::IsglNamed, size_t, auto, auto>
// NOLINTNEXTLINE(readability-identifier-naming)
struct glManagedArray;

namespace Detail {
	// NOLINTNEXTLINE(readability-identifier-naming)
	class glNamed {
	public:
		glNamed() = default;
		~glNamed() = default;
		DEFAULT_MOVE_NO_COPY(glNamed);

		GLuint
		operator*() const noexcept
		{
			return name;
		}

		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		operator GLuint() const noexcept
		{
			return name;
		}

	protected:
		GLuint name {};
		template<Detail::IsglNamed, size_t, auto, auto> friend struct ::glManagedArray;
	};
}

// NOLINTNEXTLINE(readability-identifier-naming)
template<Detail::IsglNamed Named, auto Gen, auto Del> struct glManagedSingle : public Named {
	glManagedSingle() noexcept
	{
		(*Gen)(1, &this->name);
	}

	glManagedSingle(glManagedSingle && src) noexcept
	{
		this->name = std::exchange(src.name, 0);
	}

	~glManagedSingle() noexcept
	{
		if (this->name) {
			(*Del)(1, &this->name);
		}
	}

	NO_COPY(glManagedSingle);

	glManagedSingle &
	operator=(glManagedSingle && src) noexcept
	{
		if (this->name) {
			(*Del)(1, &this->name);
		}
		this->name = std::exchange(src.name, 0);
		return *this;
	}
};

template<Detail::IsglNamed Named, size_t N, auto Gen, auto Del>
// NOLINTNEXTLINE(readability-identifier-naming)
struct glManagedArray : public std::array<Named, N> {
	using Arr = std::array<Named, N>;

	glManagedArray() noexcept
	{
		(*Gen)(N, &Arr::front().name);
	}

	glManagedArray(glManagedArray && src) noexcept
	{
		Arr::operator=(std::exchange(static_cast<Arr &>(src), {}));
	}

	~glManagedArray() noexcept
	{
		if (Arr::front().name) {
			(*Del)(N, &Arr::front().name);
		}
	}

	NO_COPY(glManagedArray);

	glManagedArray &
	operator=(glManagedArray && src) noexcept
	{
		if (Arr::front().name) {
			(*Del)(N, &Arr::front().name);
		}
		Arr::operator=(std::exchange(static_cast<Arr &>(src), {}));
		return *this;
	}

	[[nodiscard]] static constexpr size_t
	size() noexcept
	{
		return N;
	}
};

// NOLINTBEGIN(readability-identifier-naming)
template<size_t N> using glVertexArrays = glManagedArray<Detail::glNamed, N, &glGenVertexArrays, &glDeleteVertexArrays>;
using glVertexArray = glManagedSingle<Detail::glNamed, &glGenVertexArrays, &glDeleteVertexArrays>;
template<size_t N> using glBuffers = glManagedArray<Detail::glNamed, N, &glGenBuffers, &glDeleteBuffers>;
using glBuffer = glManagedSingle<Detail::glNamed, &glGenBuffers, &glDeleteBuffers>;
template<size_t N> using glFrameBuffers = glManagedArray<Detail::glNamed, N, &glGenFramebuffers, &glDeleteFramebuffers>;
using glFrameBuffer = glManagedSingle<Detail::glNamed, &glGenFramebuffers, &glDeleteFramebuffers>;
template<size_t N>
using glRenderBuffers = glManagedArray<Detail::glNamed, N, &glGenRenderbuffers, &glDeleteRenderbuffers>;
using glRenderBuffer = glManagedSingle<Detail::glNamed, &glGenRenderbuffers, &glDeleteRenderbuffers>;
// NOLINTEND(readability-identifier-naming)
