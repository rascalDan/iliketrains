#pragma once

#include <array>

template<typename T, std::size_t first, std::size_t second>
std::array<T, first + second>
operator+(const std::array<T, first> & a, const std::array<T, second> & b)
{
	std::array<T, first + second> r;
	auto out = r.begin();
	out = std::copy(a.begin(), a.end(), out);
	std::copy(b.begin(), b.end(), out);
	return r;
}
