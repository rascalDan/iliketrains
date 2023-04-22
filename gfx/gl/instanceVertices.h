#pragma once

#include "glArrays.h"
#include <iterator>
#include <span>
#include <special_members.hpp>
#include <utility>
#include <vector>

template<typename T> class InstanceVertices {
public:
	InstanceVertices(size_t initialSize = 16)
	{
		allocBuffer(initialSize);
	}

	class [[nodiscard]] InstanceProxy {
	public:
		InstanceProxy(InstanceVertices * iv, std::size_t idx) : instances {iv}, index {idx} { }
		InstanceProxy(InstanceProxy && other) : instances {std::exchange(other.instances, nullptr)}, index {other.index}
		{
		}
		NO_COPY(InstanceProxy);

		~InstanceProxy()
		{
			if (instances) {
				instances->release(index);
			}
		}

		InstanceProxy &
		operator=(InstanceProxy && other)
		{
			if (instances) {
				instances->release(index);
			}
			instances = std::exchange(other.instances, nullptr);
			index = other.index;
			return *this;
		}
		template<typename U>
		T &
		operator=(U && v)
		{
			return instances->at(index) = std::forward<U>(v);
		}

		[[nodiscard]]
		operator T &()
		{
			return instances->at(index);
		}
		[[nodiscard]] operator const T &() const
		{
			return instances->at(index);
		}
		[[nodiscard]] T *
		get()
		{
			return &instances->at(index);
		}
		[[nodiscard]] const T *
		get() const
		{
			return &instances->at(index);
		}
		[[nodiscard]] T *
		operator->()
		{
			return get();
		}
		[[nodiscard]] const T *
		operator->() const
		{
			return get();
		}
		[[nodiscard]] T &
		operator*()
		{
			return instances->at(index);
		}
		[[nodiscard]] const T &
		operator*() const
		{
			return instances->at(index);
		}

	private:
		InstanceVertices<T> * instances;
		std::size_t index;
	};

	template<typename... Params>
	[[nodiscard]] InstanceProxy
	acquire(Params &&... params)
	{
		map();
		if (!unused.empty()) {
			auto idx = unused.back();
			unused.pop_back();
			index[idx] = next++;
			new (&at(idx)) T(std::forward<Params>(params)...);
			return InstanceProxy {this, idx};
		}
		if (next >= capacity) {
			resize(capacity * 2);
		}
		index.emplace_back(next++);
		new (data + index.back()) T(std::forward<Params>(params)...);
		return InstanceProxy {this, index.size() - 1};
	}

	[[nodiscard]] const auto &
	bufferName() const
	{
		return buffer;
	}

	[[nodiscard]] auto
	count() const
	{
		unmap();
		return next;
	}

protected:
	friend InstanceProxy;

	void
	release(const size_t pidx)
	{
		// Destroy p's object
		at(pidx).~T();
		if (--next != index[pidx]) {
			// Move last object into p's slot
			new (&at(pidx)) T {std::move(data[next])};
			(data[next]).~T();
			*std::find_if(index.begin(), index.end(), [this](const auto & i) {
				const auto n = &i - index.data();
				return i == next && std::find(unused.begin(), unused.end(), n) == unused.end();
			}) = index[pidx];
		}
		if (pidx == index.size() - 1) {
			index.pop_back();
		}
		else {
			// Remember p.index is free index now
			unused.push_back(pidx);
		}
	}

	void
	allocBuffer(std::size_t newCapacity)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(T) * newCapacity), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		capacity = newCapacity;
		data = nullptr;
	}

	void
	resize(size_t newCapacity)
	{
		const auto maintain = std::min(newCapacity, capacity);
		std::vector<T> existing;
		const auto maintaind = static_cast<typename decltype(existing)::difference_type>(maintain);
		existing.reserve(maintain);
		map();
		std::move(data, data + maintain, std::back_inserter(existing));
		allocBuffer(newCapacity);
		map();
		std::move(existing.begin(), existing.begin() + maintaind, data);
		capacity = newCapacity;
	}

	[[nodiscard]] T &
	at(size_t pindex)
	{
		map();
		return data[index[pindex]];
	}

	void
	map() const
	{
		if (!data) {
			data = static_cast<T *>(glMapNamedBuffer(buffer, GL_READ_WRITE));
		}
	}

	void
	unmap() const
	{
		if (data) {
			glUnmapNamedBuffer(buffer);
			data = nullptr;
		}
	}

	glBuffer buffer;
	mutable T * data {};
	// Size of buffer
	std::size_t capacity {};
	// # used of capacity
	std::size_t next {};
	// Index into buffer given to nth proxy
	std::vector<size_t> index;
	// List of free spaces in index
	std::vector<size_t> unused;
};
