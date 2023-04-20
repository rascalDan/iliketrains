#pragma once

#include "glArrays.h"
#include <iterator>
#include <span>
#include <special_members.hpp>
#include <utility>
#include <vector>

template<typename T> class InstanceVertices {
public:
	InstanceVertices(size_t initialSize = 16) : data {allocBuffer(buffer, initialSize)}, next {} { }

	~InstanceVertices()
	{
		glUnmapNamedBuffer(buffer);
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
			return instances->data[index] = std::forward<U>(v);
		}

		T *
		get()
		{
			return &instances->data[index];
		}
		const T *
		get() const
		{
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
		if (!unused.empty()) {
			auto idx = unused.back();
			unused.pop_back();
			new (&data[idx]) T(std::forward<Params>(params)...);
			return InstanceProxy {this, idx};
		}
		if (next >= data.size()) {
			resize(data.size() * 2);
		}
		new (&data[next]) T(std::forward<Params>(params)...);
		return InstanceProxy {this, next++};
	}

	void
	release(const InstanceProxy & p)
	{
		data[p.index].~T();
		unused.push_back(p.index);
	}

	const auto &
	bufferName() const
	{
		return buffer;
	}

	auto
	count() const
	{
		return next;
	}

protected:
	friend InstanceProxy;

	static std::span<T>
	allocBuffer(const glBuffer & buffer, std::size_t count)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(T) * count), nullptr, GL_DYNAMIC_DRAW);
		auto data = static_cast<T *>(glMapNamedBuffer(buffer, GL_READ_WRITE));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return {data, count};
	}

	void
	resize(size_t newCapacity)
	{
		const auto maintain = std::min(newCapacity, data.size());
		const auto maintaind = static_cast<typename decltype(data)::difference_type>(maintain);
		std::vector<T> existing;
		existing.reserve(maintain);
		std::move(data.begin(), data.begin() + maintaind, std::back_inserter(existing));
		data = allocBuffer(buffer, newCapacity);
		std::move(existing.begin(), existing.begin() + maintaind, data.begin());
	}

	glBuffer buffer;
	std::span<T> data;
	std::size_t next;
	std::vector<size_t> unused;
};
