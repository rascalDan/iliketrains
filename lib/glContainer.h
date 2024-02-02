#pragma once

#include "glArrays.h"
#include <cassert>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

static_assert(GL_READ_ONLY < GL_READ_WRITE);
static_assert(GL_WRITE_ONLY < GL_READ_WRITE);

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
		map(GL_READ_WRITE);
		return mkspan().begin();
	}

	[[nodiscard]] iterator
	end()
	{
		map(GL_READ_WRITE);
		return mkspan().end();
	}

	[[nodiscard]] const_iterator
	begin() const
	{
		map(GL_READ_ONLY);
		return mkcspan().begin();
	}

	[[nodiscard]] const_iterator
	end() const
	{
		map(GL_READ_ONLY);
		return mkcspan().end();
	}

	[[nodiscard]] const_iterator
	cbegin() const
	{
		map(GL_READ_ONLY);
		return mkcspan().begin();
	}

	[[nodiscard]] const_iterator
	cend() const
	{
		map(GL_READ_ONLY);
		return mkcspan().end();
	}

	[[nodiscard]] reverse_iterator
	rbegin()
	{
		map(GL_READ_WRITE);
		return mkspan().rbegin();
	}

	[[nodiscard]] reverse_iterator
	rend()
	{
		map(GL_READ_WRITE);
		return mkspan().rend();
	}

	[[nodiscard]] const_reverse_iterator
	rbegin() const
	{
		map(GL_READ_ONLY);
		return mkcspan().rbegin();
	}

	[[nodiscard]] const_reverse_iterator
	rend() const
	{
		map(GL_READ_ONLY);
		return mkcspan().rend();
	}

	[[nodiscard]] const_reverse_iterator
	crbegin() const
	{
		map(GL_READ_ONLY);
		return mkcspan().rbegin();
	}

	[[nodiscard]] const_reverse_iterator
	crend() const
	{
		map(GL_READ_ONLY);
		return mkcspan().rend();
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
		if (data_) {
			mkspan()[pos] = value;
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
		map(GL_READ_WRITE);
		return mkspan()[pos];
	}

	[[nodiscard]] const_reference_type
	at(size_type pos) const
	{
		if (pos >= size()) {
			throw std::out_of_range {__FUNCTION__};
		}
		map(GL_READ_ONLY);
		return mkcspan()[pos];
	}

	[[nodiscard]] reference_type
	operator[](size_type pos)
	{
		map(GL_READ_WRITE);
		return mkspan()[pos];
	}

	[[nodiscard]] const_reference_type
	operator[](size_type pos) const
	{
		map(GL_READ_ONLY);
		return mkcspan()[pos];
	}

	[[nodiscard]] pointer_type
	data()
	{
		map(GL_READ_WRITE);
		return data_;
	}

	[[nodiscard]] const_pointer_type
	data() const
	{
		map(GL_READ_ONLY);
		return data_;
	}

	[[nodiscard]] reference_type
	front()
	{
		map(GL_READ_WRITE);
		return mkspan().front();
	}

	[[nodiscard]] reference_type
	back()
	{
		map(GL_READ_WRITE);
		return mkspan().back();
	}

	[[nodiscard]] const_reference_type
	front() const
	{
		map(GL_READ_ONLY);
		return mkcspan().front();
	}

	[[nodiscard]] const_reference_type
	back() const
	{
		map(GL_READ_ONLY);
		return mkcspan().back();
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
			data_ = {};
			access_ = {};
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
		requires std::is_default_constructible_v<T>
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
		if (newSize > size_) {
			for (auto & uninitialised : mkspan().subspan(size_, newSize - size_)) {
				new (&uninitialised) T {};
			}
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
		map(is_trivial_dest ? GL_READ_ONLY : GL_READ_WRITE);
		std::move(begin(), end(), std::back_inserter(existing));
		allocBuffer(size_);
		map(GL_READ_WRITE);
		std::move(existing.begin(), existing.end(), begin());
	}

	void
	clear() noexcept(is_trivial_dest)
	{
		if constexpr (!is_trivial_dest) {
			map(GL_READ_WRITE);
			std::for_each(begin(), end(), [](auto && v) {
				v.~T();
			});
		}
		setSize(0);
	}

	template<typename... P>
	reference_type
	emplace_back(P &&... ps)
		requires std::is_constructible_v<T, P...>
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (&*end()) T {std::forward<P>(ps)...};
		setSize(newSize);
		return back();
	}

	template<typename... P>
	iterator
	emplace(iterator pos, P &&... ps)
		requires std::is_nothrow_constructible_v<T, P...>
	{
		auto newSize = size_ + 1;
		const auto idx = pos - begin();
		reserve(newSize);
		mapForAdd();
		pos = begin() + idx;
		std::move_backward(pos, end(), end() + 1);
		pos->~T();
		new (&*pos) T {std::forward<P>(ps)...};
		setSize(newSize);
		return pos;
	}

	reference_type
	push_back(T p)
		requires std::is_move_constructible_v<T>
	{
		auto newSize = size_ + 1;
		reserve(newSize);
		mapForAdd();
		new (&*end()) T {std::move(p)};
		setSize(newSize);
		return back();
	}

	iterator
	insert(iterator pos, T p)
		requires std::is_nothrow_move_constructible_v<T>
	{
		auto newSize = size_ + 1;
		const auto idx = pos - begin();
		reserve(newSize);
		mapForAdd();
		pos = begin() + idx;
		std::move_backward(pos, end(), end() + 1);
		pos->~T();
		new (&*pos) T {std::move(p)};
		setSize(newSize);
		return pos;
	}

	void
	pop_back()
	{
		if constexpr (!is_trivial_dest) {
			map(GL_READ_WRITE);
			back().~T();
		}
		--size_;
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
		map(GL_READ_WRITE);
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
		size_ = s;
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
		access_ = {};
	}

	void
	map(GLenum access) const
	{
		if (size_ > 0) {
			mapMode(access);
		}
	}

	void
	mapForAdd() const
	{
		if (!data_) {
			mapMode(GL_READ_WRITE);
		}
	}

	void
	mapMode(GLenum access) const
	{
		if (data_ && access_ < access) {
			unmap();
		}
		if (!data_) {
			data_ = static_cast<T *>(glMapNamedBuffer(buffer_, access));
			assert(data_);
			access_ = access;
		}
	}

	span
	mkspan() const
	{
		assert(!size_ || data_);
		return span {data_, size_};
	}

	const_span
	mkcspan() const
	{
		assert(!size_ || data_);
		return const_span {data_, size_};
	}

	glBuffer buffer_;
	std::size_t capacity_ {};
	std::size_t size_ {};
	mutable T * data_;
	mutable GLenum access_ {};
};
