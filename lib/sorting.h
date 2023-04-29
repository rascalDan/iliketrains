#pragma once

#include <glm/fwd.hpp>
#include <type_traits>

template<typename T> struct PtrSorter {
	bool
	operator()(const T & a, const T & b) const
	{
		return *a < *b;
	}
};

template<typename T, auto M> struct PtrMemberSorter : public PtrSorter<T> {
	using MT = std::decay_t<decltype((*T {}).*M)>;
	using is_transparent = MT;

	using PtrSorter<T>::operator();

	bool
	operator()(const MT & a, const T & b) const
	{
		return a < (*b).*M;
	}

	bool
	operator()(const T & a, const MT & b) const
	{
		return (*a).*M < b;
	}
};

struct CompareBy {
	glm::length_t index;

	template<typename T>
	auto
	operator()(const T & a, const T & b) const
	{
		return get(a) < get(b);
	}

	template<typename T>
	auto
	get(const T & a) const
	{
		return a[index];
	}
};
