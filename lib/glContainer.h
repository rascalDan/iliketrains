#pragma once

#include "glArrays.h"
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

template<typename I, typename Direction> class basic_glContainer_iterator {
public:
	explicit basic_glContainer_iterator(I * i) : i {i} { }

	template<typename OtherI>
	basic_glContainer_iterator(const basic_glContainer_iterator<OtherI, Direction> & other) : i {&*other}
	{
	}

	auto &
	operator++() noexcept
	{
		i = Direction {}(i, 1);
		return *this;
	}

	auto
	operator++(int) noexcept
	{
		return basic_glContainer_iterator<I, Direction> {std::exchange(i, Direction {}(i, 1))};
	}

	auto &
	operator--() noexcept
	{
		i = Direction {}(i, -1);
		return *this;
	}

	auto
	operator--(int) noexcept
	{
		return basic_glContainer_iterator<I, Direction> {std::exchange(i, Direction {}(i, -1))};
	}

	[[nodiscard]] auto
	operator-(const basic_glContainer_iterator & other) const noexcept
	{
		if constexpr (std::is_same_v<Direction, std::plus<>>) {
			return this->i - other.i;
		}
		else {
			return other.i - this->i;
		}
	}

	[[nodiscard]] auto
	operator<(const basic_glContainer_iterator & other) const noexcept
	{
		if constexpr (std::is_same_v<Direction, std::plus<>>) {
			return this->i < other.i;
		}
		else {
			return other.i < this->i;
		}
	}

	auto
	operator+(std::integral auto n) const noexcept
	{
		return basic_glContainer_iterator<I, Direction> {Direction {}(i, n)};
	}

	auto
	operator-(std::integral auto n) const noexcept
	{
		return basic_glContainer_iterator<I, Direction> {Direction {}(i, -n)};
	}

	[[nodiscard]] bool
	operator==(const basic_glContainer_iterator & other) const noexcept
	{
		return this->i == other.i;
	}

	[[nodiscard]] bool
	operator!=(const basic_glContainer_iterator & other) const noexcept
	{
		return this->i != other.i;
	}

	[[nodiscard]] auto
	operator->() const noexcept
	{
		return i;
	}

	[[nodiscard]] auto &
	operator*() const noexcept
	{
		return *i;
	}

private:
	I * i;
};

template<typename T> class glContainer {
public:
	using value_type = T;
	using reference_type = T &;
	using const_reference_type = const T &;
	using pointer_type = T *;
	using const_pointer_type = const T *;
	using size_type = std::size_t;
	using iterator = basic_glContainer_iterator<value_type, std::plus<>>;
	using const_iterator = basic_glContainer_iterator<const value_type, std::plus<>>;
	using reserve_iterator = basic_glContainer_iterator<value_type, std::minus<>>;
	using const_reserve_iterator = basic_glContainer_iterator<const value_type, std::minus<>>;
	static constexpr bool is_trivial_dest = std::is_trivially_destructible_v<T>;

	glContainer()
	{
		allocBuffer(1);
	}

	~glContainer() noexcept(is_trivial_dest)
	{
		if constexpr (!is_trivial_dest) {
			clear();
		}
	}

	DEFAULT_MOVE_NO_COPY(glContainer);

	[[nodiscard]] iterator
	begin()
	{
		map();
		return iterator {data_};
	}

	[[nodiscard]] iterator
	end()
	{
		map();
		return iterator {data_ + size_};
	}

	[[nodiscard]] const_iterator
	begin() const
	{
		map();
		return const_iterator {data_};
	}

	[[nodiscard]] const_iterator
	end() const
	{
		map();
		return const_iterator {data_ + size_};
	}

	[[nodiscard]] const_iterator
	cbegin() const
	{
		map();
		return const_iterator {data_};
	}

	[[nodiscard]] const_iterator
	cend() const
	{
		map();
		return const_iterator {data_ + size_};
	}

	[[nodiscard]] reserve_iterator
	rbegin()
	{
		map();
		return reserve_iterator {data_ + size_ - 1};
	}

	[[nodiscard]] reserve_iterator
	rend()
	{
		map();
		return reserve_iterator {data_ - 1};
	}

	[[nodiscard]] const_reserve_iterator
	rbegin() const
	{
		map();
		return const_reserve_iterator {data_ + size_ - 1};
	}

	[[nodiscard]] const_reserve_iterator
	rend() const
	{
		map();
		return const_reserve_iterator {data_ - 1};
	}

	[[nodiscard]] const_reserve_iterator
	crbegin() const
	{
		map();
		return const_reserve_iterator {data_ + size_ - 1};
	}

	[[nodiscard]] const_reserve_iterator
	crend() const
	{
		map();
		return const_reserve_iterator {data_ - 1};
	}

	[[nodiscard]] const auto &
	bufferName() const
	{
		return buffer_;
	}

	[[nodiscard]] size_type
	size() const
	{
		return size_;
	}

	[[nodiscard]] reference_type
	at(size_type pos)
	{
		if (pos >= size()) {
			throw std::out_of_range {__FUNCTION__};
		}
		map();
		return data_[pos];
	}

	[[nodiscard]] const_reference_type
	at(size_type pos) const
	{
		if (pos >= size()) {
			throw std::out_of_range {__FUNCTION__};
		}
		map();
		return data_[pos];
	}

	[[nodiscard]] reference_type
	operator[](size_type pos)
	{
		map();
		return data_[pos];
	}

	[[nodiscard]] const_reference_type
	operator[](size_type pos) const
	{
		map();
		return data_[pos];
	}

	[[nodiscard]] pointer_type
	data()
	{
		map();
		return data_;
	}

	[[nodiscard]] const_pointer_type
	data() const
	{
		map();
		return data_;
	}

	[[nodiscard]] reference_type
	front()
	{
		map();
		return *data_;
	}

	[[nodiscard]] reference_type
	back()
	{
		map();
		return *(data_ + size_ - 1);
	}

	[[nodiscard]] const_reference_type
	front() const
	{
		map();
		return *data_;
	}

	[[nodiscard]] const_reference_type
	back() const
	{
		map();
		return *(data_ + size_ - 1);
	}

	[[nodiscard]] bool
	empty() const
	{
		return !size();
	}

	[[nodiscard]] size_type
	capacity() const
	{
		return capacity_;
	}

	void
	unmap() const
	{
		if (data_) {
			glUnmapNamedBuffer(buffer_);
			data_ = nullptr;
		}
	}

	void
	reserve(size_type newCapacity)
	{
		if (newCapacity <= capacity_) {
			return;
		}
		newCapacity = std::max(newCapacity, capacity_ * 2);

		std::vector<T> existing;
		existing.reserve(size_);
		map();
		std::move(begin(), end(), std::back_inserter(existing));
		allocBuffer(newCapacity);
		map();
		std::move(existing.begin(), existing.end(), begin());
	}

	void
	resize(size_type newSize)
	{
		if (newSize == size_) {
			return;
		}

		if (const auto maintain = std::min(newSize, size_)) {
			std::vector<T> existing;
			const auto maintaind = static_cast<typename decltype(existing)::difference_type>(maintain);
			existing.reserve(maintain);
			map();
			std::move(data_, data_ + maintain, std::back_inserter(existing));
			if constexpr (!is_trivial_dest) {
				for (auto uninitialised = data_ + newSize; uninitialised < data_ + size_; ++uninitialised) {
					uninitialised->~T();
				}
			}
			allocBuffer(newSize);
			mapForAdd();
			std::move(existing.begin(), existing.begin() + maintaind, data_);
		}
		else {
			allocBuffer(newSize);
			mapForAdd();
		}
		for (auto uninitialised = data_ + size_; uninitialised < data_ + newSize; ++uninitialised) {
			new (uninitialised) T {};
		}
		size_ = newSize;
	}

	void
	shrink_to_fit()
	{
		if (capacity_ <= size_) {
			return;
		}

		std::vector<T> existing;
		existing.reserve(size_);
		map();
		std::move(begin(), end(), std::back_inserter(existing));
		allocBuffer(size_);
		map();
		std::move(existing.begin(), existing.end(), begin());
	}

	void
	clear() noexcept(is_trivial_dest)
	{
		if constexpr (!is_trivial_dest) {
			map();
			std::for_each(begin(), end(), [](auto && v) {
				v.~T();
			});
		}
		size_ = 0;
	}

	template<typename... P>
	reference_type
	emplace_back(P &&... ps)
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (data_ + size_) T {std::forward<P>(ps)...};
		size_ = newSize;
		return back();
	}

	template<typename... P>
	iterator
	emplace(iterator pos, P &&... ps)
	{
		static_assert(std::is_nothrow_constructible_v<T, P...>);
		auto newSize = size_ + 1;
		const auto idx = pos - begin();
		reserve(newSize);
		mapForAdd();
		std::move_backward(begin() + idx, end(), end() + 1);
		(data_ + idx)->~T();
		new (data_ + idx) T {std::forward<P>(ps)...};
		size_ = newSize;
		return pos;
	}

	reference_type
	push_back(T p)
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (data_ + size_) T {std::move(p)};
		size_ = newSize;
		return back();
	}

	iterator
	insert(iterator pos, T p)
	{
		static_assert(std::is_nothrow_move_constructible_v<T>);
		auto newSize = size_ + 1;
		const auto idx = pos - begin();
		reserve(newSize);
		mapForAdd();
		std::move_backward(begin() + idx, end(), end() + 1);
		(data_ + idx)->~T();
		new (data_ + idx) T {std::move(p)};
		size_ = newSize;
		return pos;
	}

	void
	pop_back()
	{
		if constexpr (!is_trivial_dest) {
			map();
			data_[--size_].~T();
		}
		else {
			--size_;
		}
	}

	void
	erase(iterator pos)
	{
		erase(pos, pos + 1);
	}

	void
	erase(iterator pos, iterator to)
	{
		const auto eraseSize = to - pos;
		map();
		std::move(to, end(), pos);
		if constexpr (!is_trivial_dest) {
			std::for_each(end() - eraseSize, end(), [](auto && v) {
				v.~T();
			});
		}
		size_ -= static_cast<size_type>(eraseSize);
	}

protected:
	void
	allocBuffer(size_type newCapacity)
	{
		if (newCapacity == 0) {
			return allocBuffer(1);
		}
		glBindBuffer(GL_ARRAY_BUFFER, buffer_);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(T) * newCapacity), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		capacity_ = newCapacity;
		data_ = nullptr;
	}

	void
	map() const
	{
		if (size_ > 0) {
			mapForAdd();
		}
	}

	void
	mapForAdd() const
	{
		if (!data_) {
			data_ = static_cast<T *>(glMapNamedBuffer(buffer_, GL_READ_WRITE));
			assert(data_);
		}
	}

	glBuffer buffer_;
	std::size_t capacity_ {};
	std::size_t size_ {};
	mutable T * data_ {};
};

template<typename T, typename D> struct std::iterator_traits<basic_glContainer_iterator<T, D>> {
	using difference_type = ssize_t;
	using value_type = T;
	using pointer = T *;
	using reference = T &;
	using iterator_category = std::random_access_iterator_tag;
};
