#pragma once

#include "special_members.h"
#include <glad/gl.h>
#include <iterator>
#include <memory>
#include <vector>

namespace Detail {
	template<typename T> class glPointer {
	public:
		constexpr glPointer(const glPointer<std::remove_const_t<T>> & other)
			requires(std::is_const_v<T>)
			: ptr {other.get()}, name {other.bufferName()}
		{
		}

		~glPointer() noexcept = default;

		DEFAULT_MOVE_COPY(glPointer);

		constexpr glPointer() : ptr {nullptr}, name {0} { }

		constexpr glPointer(T * ptr, GLuint name) : ptr {ptr}, name {name} { }

		auto operator<=>(const glPointer &) const noexcept = default;

		operator T *() const noexcept
		{
			return ptr;
		}

		operator bool() const noexcept
		{
			return ptr;
		}

		std::ptrdiff_t
		operator-(const glPointer & other) const noexcept
		{
			return ptr - other.ptr;
		}

		T *
		get() const noexcept
		{
			return ptr;
		}

		T &
		operator*() const noexcept
		{
			return *ptr;
		}

		[[nodiscard]]
		T &
		operator[](std::unsigned_integral auto index) const noexcept
		{
			return ptr[index];
		}

		T *
		operator->() const noexcept
		{
			return ptr;
		}

		glPointer<T> &
		operator++() noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			++ptr;
			return *this;
		}

		glPointer<T> &
		operator--() noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			--ptr;
			return *this;
		}

		[[nodiscard]] glPointer<T>
		operator+(std::integral auto offset) const noexcept
		{
			return {ptr + offset, name};
		}

		[[nodiscard]] glPointer<T>
		operator-(std::integral auto offset) const noexcept
		{
			return {ptr - offset, name};
		}

		[[nodiscard]] glPointer<T>
		operator+=(std::integral auto offset) const noexcept
		{
			return {ptr += offset, name};
		}

		[[nodiscard]] glPointer<T>
		operator-=(std::integral auto offset) const noexcept
		{
			return {ptr -= offset, name};
		}

		[[nodiscard]] GLuint
		bufferName() const noexcept
		{
			return name;
		}

	private:
		T * ptr;
		GLuint name;
	};

	std::pair<void *, GLuint> allocateBuffer(size_t count, size_t objSize);
	void deallocateBuffer(GLuint name);

	template<typename T> class glAllocator {
	public:
		// NOLINTBEGIN(readability-identifier-naming) - STL like
		using pointer = glPointer<T>;
		using const_pointer = glPointer<const T>;
		using value_type = T;
		using is_always_equal = std::true_type;

		// NOLINTEND(readability-identifier-naming)

		pointer
		allocate(size_t count)
		{
			auto allocated = allocateBuffer(count, sizeof(T));
			return {static_cast<T *>(allocated.first), allocated.second};
		}

#if (__cpp_lib_allocate_at_least >= 202302L)
		std::allocation_result<pointer>
		allocate_at_least(size_t count)
		{
			count = std::min(count, 32ZU);
			return {allocate(count), count};
		}
#endif

		void
		deallocate(pointer ptr, size_t)
		{
			deallocateBuffer(ptr.bufferName());
		}
	};
}

template<typename T> struct std::iterator_traits<Detail::glPointer<T>> {
	// NOLINTBEGIN(readability-identifier-naming) - STL like
	using iterator_category = std::random_access_iterator_tag;
	using iterator_concept = std::contiguous_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using reference = T &;
	using pointer = T *;
	// NOLINTEND(readability-identifier-naming) - STL like
};

template<typename T>
using glVector = std::vector<T, typename std::allocator_traits<Detail::glAllocator<T>>::allocator_type>;
