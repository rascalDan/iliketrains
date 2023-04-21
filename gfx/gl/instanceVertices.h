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
			instances = std::exchange(other.instances, nullptr);
			index = other.index;
		}

		operator T &()
		{
			return *get();
		}
		operator const T &() const
		{
			return *get();
		}
		template<typename U>
		T &
		operator=(U && v)
		{
			instances->map();
			return instances->data[index] = std::forward<U>(v);
		}

		T *
		get()
		{
			instances->map();
			return &instances->data[index];
		}
		const T *
		get() const
		{
			instances->map();
			return &instances->data[index];
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
			return *get();
		}
		const T &
		operator*() const
		{
			return *get();
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
			new (data + idx) T(std::forward<Params>(params)...);
			return InstanceProxy {this, idx};
		}
		if (next >= capacity) {
			resize(capacity * 2);
		}
		new (data + next) T(std::forward<Params>(params)...);
		return InstanceProxy {this, next++};
	}

	void
	release(const InstanceProxy & p)
	{
		map();
		data[p.index].~T();
		unused.push_back(p.index);
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
	std::size_t capacity {};
	std::size_t next {};
	std::vector<size_t> unused;
};
