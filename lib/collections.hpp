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

template<typename T, std::size_t N>
auto
operator*(const std::array<T, N> & in, auto && f)
{
	std::array<decltype(f(in[0])), N> out;

	for (auto outitr = out.begin(); const auto & v : in) {
		*outitr++ = f(v);
	}
	return out;
}

template<typename T, std::size_t N>
auto &
operator*=(std::array<T, N> & in, auto && f)
{
	for (const auto & v : in) {
		f(v);
	}
}
