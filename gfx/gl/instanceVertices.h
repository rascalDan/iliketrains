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

	class InstanceProxy {
	public:
		InstanceProxy(InstanceVertices * iv, std::size_t idx) : instances {iv}, index {idx} { }
		InstanceProxy(InstanceProxy && other) : instances {std::exchange(other.instances, nullptr)}, index {other.index}
		{
		}
		NO_COPY(InstanceProxy);

		~InstanceProxy()
		{
			if (instances) {
				instances->release(*this);
			}
		}

		InstanceProxy &
		operator=(InstanceProxy && other)
		{
			if (instances) {
				instances->release(*this);
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

		operator T &()
		{
			return instances->at(index);
		}
		operator const T &() const
		{
			return instances->at(index);
		}

		T *
		get()
		{
			return &instances->at(index);
		}
		const T *
		get() const
		{
			return &instances->at(index);
		}
		T *
		operator->()
		{
			return get();
		}
		const T *
		operator->() const
		{
			return get();
		}
		T &
		operator*()
		{
			return instances->at(index);
		}
		const T &
		operator*() const
		{
			return instances->at(index);
		}

	private:
		friend InstanceVertices;

		InstanceVertices<T> * instances;
		std::size_t index;
	};

	template<typename... Params>
	InstanceProxy
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

	const auto &
	bufferName() const
	{
		unmap();
		return buffer;
	}

	auto
	count() const
	{
		unmap();
		return next;
	}

protected:
	friend InstanceProxy;

	void
	release(const InstanceProxy & p)
	{
		// Destroy p's object
		at(p.index).~T();
		if (next-- > index[p.index]) {
			// Remember p.index is free index now
			unused.push_back(p.index);
			// Move last object into p's slot
			new (&at(p.index)) T {std::move(data[next])};
			(data[next]).~T();
			for (auto & i : index) {
				if (i == next) {
					i = index[p.index];
					break;
				}
			}
			index[p.index] = 100000;
		}
		else {
			index.pop_back();
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

	T &
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
