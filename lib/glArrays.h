#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <glad/gl.h>
#include <special_members.h>
#include <utility>

namespace Detail {
	class glNamed;

	template<typename Named>
	concept IsglNamed = sizeof(Named) == sizeof(GLuint) && std::is_base_of_v<Detail::glNamed, Named>;
}

template<Detail::IsglNamed, size_t, auto, auto> struct glManagedArray;

namespace Detail {
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

template<Detail::IsglNamed Named, size_t N, auto Gen, auto Del> struct glManagedArray : public std::array<Named, N> {
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
