#pragma once

#include "glContainer.h"
#include "pack.h"
#include <cassert>
#include <special_members.h>
#include <utility>

template<typename T> class InstanceVertices : protected glContainer<T> {
	using base = glContainer<T>;

public:
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
			return instances->lookup(index) = std::forward<U>(v);
		}

		[[nodiscard]] operator T &()
		{
			return instances->lookup(index);
		}

		[[nodiscard]] operator const T &() const
		{
			return instances->lookup(index);
		}

		[[nodiscard]] T *
		get()
		{
			return &instances->lookup(index);
		}

		[[nodiscard]] const T *
		get() const
		{
			return &instances->lookup(index);
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
			return instances->lookup(index);
		}

		[[nodiscard]] const T &
		operator*() const
		{
			return instances->lookup(index);
		}

	private:
		InstanceVertices<T> * instances;
		std::size_t index;
	};

	template<typename... Params>
	[[nodiscard]] InstanceProxy
	acquire(Params &&... params)
	{
		if (!unused.empty()) {
			auto idx = unused.back();
			unused.pop_back();
			index[idx] = base::size();
			base::emplace_back(std::forward<Params>(params)...);
			return InstanceProxy {this, idx};
		}
		index.emplace_back(base::size());
		base::emplace_back(std::forward<Params>(params)...);
		return InstanceProxy {this, index.size() - 1};
	}

	using base::bufferName;

	[[nodiscard]] auto
	size() const
	{
		base::unmap();
		return base::size();
	}

protected:
	friend InstanceProxy;

	void
	release(const size_t pidx)
	{
		if (base::size() - 1 != index[pidx]) {
			lookup(pidx) = std::move(base::back());
			*std::find_if(index.begin(), index.end(), [this, old = base::size() - 1](const auto & i) {
				return i == old && !std::binary_search(unused.begin(), unused.end(), &i - index.data());
			}) = index[pidx];
		}
		base::pop_back();
		if (pidx == index.size() - 1) {
			index.pop_back();
		}
		else {
			// Remember p.index is free index now, keeping it sorted
			unused.insert(std::upper_bound(unused.begin(), unused.end(), pidx), pidx);
		}
	}

	[[nodiscard]] T &
	lookup(size_t pindex)
	{
		return base::data()[index[pindex]];
	}

	//  Index into buffer given to nth proxy
	std::vector<size_t> index;
	//  List of free spaces in index
	std::vector<size_t> unused;
};
