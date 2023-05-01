#pragma once

#include <utility>

template<typename T, template<typename... S> typename Container> class pack : protected Container<T> {
public:
	using Container<T>::Container;

	using Container<T>::begin;
	using Container<T>::end;
	using Container<T>::rbegin;
	using Container<T>::rend;
	using Container<T>::cbegin;
	using Container<T>::cend;
	using Container<T>::crbegin;
	using Container<T>::crend;
	using Container<T>::clear;
	using Container<T>::empty;
	using Container<T>::size;
	using Container<T>::capacity;
	using Container<T>::shrink_to_fit;
	using Container<T>::at;
	using Container<T>::data;
	using Container<T>::operator[];

	template<typename... Ps>
	decltype(auto)
	emplace(Ps &&... ps)
	{
		return Container<T>::emplace_back(std::forward<Ps>(ps)...);
	}

	void
	erase(typename Container<T>::iterator pos)
	{
		pos->~T();
		if (&*pos != &Container<T>::back()) {
			new (&*pos) T(std::move(Container<T>::back()));
		}
		Container<T>::pop_back();
	}
};
