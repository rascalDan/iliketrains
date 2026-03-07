#pragma once

#include "glAllocator.h"
#include <cassert>
#include <functional>
#include <special_members.h>
#include <utility>

template<typename T> class InstanceVertices : protected glVector<T> {
	using base = glVector<T>;

public:
	class [[nodiscard]] InstanceProxy {
	public:
		InstanceProxy(InstanceVertices * iv, std::size_t idx) : instances {iv}, index {idx} { }

		InstanceProxy(InstanceProxy && other) noexcept :
			instances {std::exchange(other.instances, nullptr)}, index {other.index}
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
		operator=(InstanceProxy && other) noexcept
		{
			if (instances) {
				instances->release(index);
			}
			instances = std::exchange(other.instances, nullptr);
			index = other.index;
			return *this;
		}

		template<typename U>
		InstanceProxy &
		operator=(U && v)
		{
			instances->lookup(index) = std::forward<U>(v);
			return *this;
		}

		// NOLINTNEXTLINE)hicpp-explicit-conversions
		[[nodiscard]]
		operator T &()
		{
			return instances->lookup(index);
		}

		// NOLINTNEXTLINE)hicpp-explicit-conversions
		[[nodiscard]]
		operator const T &() const
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
			reverseIndex.emplace_back(idx);
			base::emplace_back(std::forward<Params>(params)...);
			return InstanceProxy {this, idx};
		}
		index.emplace_back(base::size());
		reverseIndex.push_back(base::size());
		base::emplace_back(std::forward<Params>(params)...);
		return InstanceProxy {this, index.size() - 1};
	}

	[[nodiscard]] GLuint
	bufferName() const
	{
		return base::get_allocator().getNameFor(static_cast<const base &>(*this));
	}

	using base::value_type;

	using base::at;
	using base::begin;
	using base::cbegin;
	using base::cend;
	using base::crbegin;
	using base::crend;
	using base::end;
	using base::rbegin;
	using base::rend;
	using base::size;
	using base::operator[];
	using base::back;
	using base::capacity;
	using base::data;
	using base::empty;
	using base::front;
	using base::reserve;
	using base::shrink_to_fit;

	template<typename Pred>
	base::size_type
	partition(Pred pred)
	{
		return indexOf(partition(base::begin(), base::end(), pred));
	}

	using PartitionResult
			= std::pair<typename base::size_type, std::pair<typename base::size_type, typename base::size_type>>;

	template<typename Pred1, typename Pred2>
	PartitionResult
	partition(Pred1 pred1, Pred2 pred2)
	{
		return partition(pred1, std::not_fn(pred2), pred2);
	}

	template<typename Pred1, typename Pred2, typename Pred3>
	PartitionResult
	partition(Pred1 pred1, Pred2 pred2, Pred3 pred3)
	{
		auto boundary1 = partition(base::begin(), base::end(), pred1);
		auto begin2 = partition(base::begin(), boundary1, pred2);
		auto end2 = partition(boundary1, base::end(), pred3);
		return {indexOf(boundary1), {indexOf(begin2), indexOf(end2)}};
	}

protected:
	static constexpr auto npos = static_cast<size_t>(-1);
	friend InstanceProxy;

	base::size_type
	indexOf(base::iterator iter)
	{
		return static_cast<base::size_type>(iter - base::begin());
	}

	void
	release(const size_t pidx)
	{
		if (const size_t last = base::size() - 1; last != index[pidx]) {
			lookup(pidx) = std::move(base::back());
			const auto movedKey = reverseIndex[last];
			index[movedKey] = std::exchange(index[pidx], npos);
			reverseIndex[index[movedKey]] = movedKey;
		}
		base::pop_back();
		reverseIndex.pop_back();
		if (pidx == index.size() - 1) {
			index.pop_back();
		}
		else {
			index[pidx] = npos;
			// Remember p.index is free index now
			unused.emplace_back(pidx);
		}
	}

	[[nodiscard]] T &
	lookup(size_t pindex)
	{
		return base::data()[index[pindex]];
	}

	template<typename Pred>
	base::iterator
	partition(base::iterator first, base::iterator last, Pred pred)
	{
		while (first < last) {
			first = std::find_if_not(first, last, pred);
			last = --std::find_if(std::make_reverse_iterator(last), std::make_reverse_iterator(first), pred).base();
			if (first < last) {
				std::iter_swap(first, last);
				const auto fidx = static_cast<size_t>(first - base::begin()),
						   lidx = static_cast<size_t>(last - base::begin());
				std::swap(index[reverseIndex[fidx]], index[reverseIndex[lidx]]);
				std::swap(reverseIndex[fidx], reverseIndex[lidx]);
			}
		}
		return first;
	}

	//  Index into buffer given to nth proxy
	std::vector<size_t> index;
	std::vector<size_t> reverseIndex;
	//  List of free spaces in index
	std::vector<size_t> unused;
};
