#ifndef SORTING_H
#define SORTING_H

template<typename T> struct PtrSorter {
	bool
	operator()(const T & a, const T & b) const
	{
		return *a < *b;
	}
};

#endif
