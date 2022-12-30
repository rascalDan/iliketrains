#pragma once

#include <array>
#include <span>

template<typename T, std::size_t first, std::size_t second>
constexpr std::array<T, first + second>
operator+(const std::array<T, first> & a, const std::array<T, second> & b)
{
	std::array<T, first + second> r;
	auto out = r.begin();
	out = std::copy(a.begin(), a.end(), out);
	std::copy(b.begin(), b.end(), out);
	return r;
}

template<typename T, typename V, std::size_t first, std::size_t second>
constexpr std::array<std::pair<T, V>, first * second>
operator*(const std::array<T, first> & a, const std::array<V, second> & b)
{
	std::array<std::pair<T, V>, first * second> r;
	auto out = r.begin();
	for (const auto & ae : a) {
		for (const auto & be : b) {
			*out++ = {ae, be};
		}
	}
	return r;
}

template<typename T, std::size_t N>
constexpr auto
operator*(const std::array<T, N> & in, auto && f)
{
	std::array<decltype(f(in[0])), N> out;

	for (auto outitr = out.begin(); const auto & v : in) {
		*outitr++ = f(v);
	}
	return out;
}

template<typename T>
constexpr auto &
operator*=(std::span<T> & in, auto && f)
{
	for (auto & v : in) {
		f(v);
	}
	return in;
}
