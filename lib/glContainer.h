#pragma once

#include "glArrays.h"
#include <cassert>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

template<typename T> class glContainer {
public:
	using span = std::span<T>;
	using const_span = std::span<const T>;
	using value_type = T;
	using reference_type = T &;
	using const_reference_type = const T &;
	using pointer_type = T *;
	using const_pointer_type = const T *;
	using size_type = std::size_t;
	using iterator = span::iterator;
	using const_iterator = const_span::iterator;
	using reverse_iterator = span::reverse_iterator;
	using const_reverse_iterator = const_span::reverse_iterator;
	static constexpr bool is_trivial_dest = std::is_trivially_destructible_v<T>;

	glContainer()
	{
		allocBuffer(1);
	}

	~glContainer()
		requires(is_trivial_dest)
	= default;

	~glContainer()
		requires(!is_trivial_dest)
	{
		clear();
	}

	template<template<typename, typename...> typename C> explicit glContainer(const C<T> & src)
	{
		reserve(src.size());
		std::copy(src.begin(), src.end(), std::back_inserter(*this));
	}

	DEFAULT_MOVE_NO_COPY(glContainer);

	[[nodiscard]] iterator
	begin()
	{
		map();
		return data_.begin();
	}

	[[nodiscard]] iterator
	end()
	{
		map();
		return data_.end();
	}

	[[nodiscard]] const_iterator
	begin() const
	{
		map();
		return const_span {data_}.begin();
	}

	[[nodiscard]] const_iterator
	end() const
	{
		map();
		return const_span {data_}.end();
	}

	[[nodiscard]] const_iterator
	cbegin() const
	{
		map();
		return const_span {data_}.begin();
	}

	[[nodiscard]] const_iterator
	cend() const
	{
		map();
		return const_span {data_}.end();
	}

	[[nodiscard]] reverse_iterator
	rbegin()
	{
		map();
		return data_.rbegin();
	}

	[[nodiscard]] reverse_iterator
	rend()
	{
		map();
		return data_.rend();
	}

	[[nodiscard]] const_reverse_iterator
	rbegin() const
	{
		map();
		return const_span {data_}.rbegin();
	}

	[[nodiscard]] const_reverse_iterator
	rend() const
	{
		map();
		return const_span {data_}.rend();
	}

	[[nodiscard]] const_reverse_iterator
	crbegin() const
	{
		map();
		return const_span {data_}.rbegin();
	}

	[[nodiscard]] const_reverse_iterator
	crend() const
	{
		map();
		return const_span {data_}.rend();
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

	void
	at(size_type pos, const T & value)
	{
		if (pos >= size()) {
			throw std::out_of_range {__FUNCTION__};
		}
		if (data_.data()) {
			data_[pos] = value;
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(pos * sizeof(T)), sizeof(value), &value);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
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
		return data_.data();
	}

	[[nodiscard]] const_pointer_type
	data() const
	{
		map();
		return data_.data();
	}

	[[nodiscard]] reference_type
	front()
	{
		map();
		return data_.front();
	}

	[[nodiscard]] reference_type
	back()
	{
		map();
		return data_.back();
	}

	[[nodiscard]] const_reference_type
	front() const
	{
		map();
		return data_.front();
	}

	[[nodiscard]] const_reference_type
	back() const
	{
		map();
		return data_.back();
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
		if (data_.data()) {
			glUnmapNamedBuffer(buffer_);
			data_ = {};
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
		std::move(begin(), end(), std::back_inserter(existing));
		allocBuffer(newCapacity);
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
			std::move(begin(), end(), std::back_inserter(existing));
			allocBuffer(newSize);
			mapForAdd();
			std::move(existing.begin(), existing.begin() + maintaind, begin());
		}
		else {
			allocBuffer(newSize);
			mapForAdd();
		}
		for (auto uninitialised = data_.data() + size_; uninitialised < data_.data() + newSize; ++uninitialised) {
			new (uninitialised) T {};
		}
		setSize(newSize);
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
		setSize(0);
	}

	template<typename... P>
	reference_type
	emplace_back(P &&... ps)
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (&data_[size_]) T {std::forward<P>(ps)...};
		setSize(newSize);
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
		auto newT = begin() + idx;
		std::move_backward(newT, end(), end() + 1);
		newT->~T();
		new (&*newT) T {std::forward<P>(ps)...};
		setSize(newSize);
		return pos;
	}

	reference_type
	push_back(T p)
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (&data_[size_]) T {std::move(p)};
		setSize(newSize);
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
		auto newT = begin() + idx;
		std::move_backward(newT, end(), end() + 1);
		newT->~T();
		new (&*newT) T {std::move(p)};
		setSize(newSize);
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
		setSize(size_ - static_cast<size_type>(eraseSize));
	}

protected:
	void
	setSize(size_type s)
	{
		data_ = {data_.data(), size_ = s};
	}

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
		data_ = {};
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
		if (!data_.data()) {
			data_ = {static_cast<T *>(glMapNamedBuffer(buffer_, GL_READ_WRITE)), size_};
			assert(data_.data());
		}
	}

	glBuffer buffer_;
	std::size_t capacity_ {};
	std::size_t size_ {};
	mutable span data_;
};
