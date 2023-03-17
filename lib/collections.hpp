#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <utility>
#include <vector>

template<typename T, typename E>
concept SequentialCollection = requires(T c) {
								   {
									   c.size()
									   } -> std::integral;
								   {
									   c.data()
									   } -> std::same_as<const E *>;
							   };
template<typename T>
concept IterableCollection = std::is_same_v<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>;

template<typename T, std::size_t first, std::size_t second>
[[nodiscard]] constexpr std::array<T, first + second>
operator+(const std::array<T, first> & a, const std::array<T, second> & b)
{
	std::array<T, first + second> r;
	auto out = r.begin();
	out = std::copy(a.begin(), a.end(), out);
	std::copy(b.begin(), b.end(), out);
	return r;
}

template<typename T, typename V, std::size_t first, std::size_t second>
[[nodiscard]] constexpr std::array<std::pair<T, V>, first * second>
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
[[nodiscard]] constexpr auto
operator*(const std::array<T, N> & in, auto && f)
{
	std::array<decltype(f(in[0])), N> out;

	for (auto outitr = out.begin(); const auto & v : in) {
		*outitr++ = f(v);
	}
	return out;
}

constexpr auto &
operator*=(IterableCollection auto & in, auto && f)
{
	for (auto & v : in) {
		f(v);
	}
	return in;
}

template<template<typename...> typename C, typename... T>
[[nodiscard]] constexpr auto
operator*(const C<T...> & in, auto && f)
{
	C<decltype(f(in[0]))> out;

	std::transform(in.begin(), in.end(), std::inserter(out, out.end()), f);
	return out;
}

template<typename... T>
constexpr auto &
operator+=(std::vector<T...> & in, std::vector<T...> && src)
{
	in.reserve(in.size() + src.size());
	std::move(src.begin(), src.end(), std::back_inserter(in));
	return in;
}

template<typename... T, typename Vn>
[[nodiscard]] constexpr auto
operator+(const std::vector<T...> & in, Vn && vn)
{
	auto out(in);
	out.emplace_back(std::forward<Vn>(vn));
	return out;
}

template<template<typename> typename Direction = std::plus, typename T = unsigned int>
[[nodiscard]] static auto
vectorOfN(std::integral auto N, T start = {}, T step = 1)
{
	std::vector<T> v;
	v.resize(N);
	std::generate_n(v.begin(), N, [&start, step, adj = Direction {}]() {
		return std::exchange(start, adj(start, step));
	});
	return v;
}

template<template<typename...> typename Rtn = std::vector, IterableCollection In>
[[nodiscard]] auto
materializeRange(In && in)
{
	return Rtn(in.begin(), in.end());
}

template<template<typename...> typename Rtn = std::vector, typename In>
[[nodiscard]] auto
materializeRange(const std::pair<In, In> & in)
{
	return Rtn(in.first, in.second);
}
