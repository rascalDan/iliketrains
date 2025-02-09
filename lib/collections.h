#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

template<typename T, typename E>
concept SequentialCollection = requires(T c) {
	{ c.size() } -> std::integral;
	{ c.data() } -> std::same_as<const E *>;
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
	requires IterableCollection<C<T...>>
[[nodiscard]] constexpr auto
operator*(const C<T...> & in, auto && f)
{
	C<decltype(f(*in.begin()))> out;
	if constexpr (requires { out.reserve(in.size()); }) {
		out.reserve(in.size());
	}

	std::transform(in.begin(), in.end(), std::inserter(out, out.end()), f);
	return out;
}

template<typename T, std::size_t N>
[[nodiscard]] constexpr auto
operator*(const std::span<T, N> in, auto && f)
{
	std::array<decltype(f(std::declval<T>())), N> out;

	std::transform(in.begin(), in.end(), out.begin(), f);
	return out;
}

template<typename T>
[[nodiscard]] constexpr auto
operator*(const std::span<T, std::dynamic_extent> in, auto && f)
{
	std::vector<decltype(f(std::declval<T>()))> out;

	out.reserve(in.size());
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

template<typename... T>
constexpr auto
operator+(std::vector<T...> in1, std::vector<T...> in2)
{
	in1.reserve(in1.size() + in2.size());
	std::move(in2.begin(), in2.end(), std::back_inserter(in1));
	return in1;
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

template<template<typename...> typename Rtn = std::vector, typename In>
[[nodiscard]] auto
materializeRange(const In begin, const In end)
{
	return Rtn(begin, end);
}

template<template<typename...> typename Rtn = std::vector, IterableCollection In>
[[nodiscard]] auto
materializeRange(const In & in)
{
	return materializeRange<Rtn>(in.begin(), in.end());
}

template<template<typename...> typename Rtn = std::vector, typename In>
[[nodiscard]] auto
materializeRange(const std::pair<In, In> & in)
{
	return materializeRange<Rtn>(in.first, in.second);
}

template<typename T> struct pair_range {
	constexpr auto &
	begin() const noexcept
	{
		return pair.first;
	}

	constexpr auto &
	end() const noexcept
	{
		return pair.second;
	}

	const std::pair<T, T> & pair;
};

template<typename T> pair_range(std::pair<T, T>) -> pair_range<T>;

template<typename iter> struct stripiter {
	[[nodiscard]] constexpr bool
	operator!=(const stripiter & other) const
	{
		return current != other.current;
	}

	[[nodiscard]] constexpr bool
	operator==(const stripiter & other) const
	{
		return current == other.current;
	}

	constexpr stripiter &
	operator++()
	{
		++current;
		off = 1 - off;
		return *this;
	}

	constexpr stripiter &
	operator--()
	{
		--current;
		off = 1 - off;
		return *this;
	}

	constexpr auto
	operator-(const stripiter & other) const
	{
		return current - other.current;
	}

	constexpr auto
	operator*() const
	{
		return std::tie(*(current - (2 - off)), *(current - off - 1), *current);
	}

	iter current;
	uint8_t off {};
};

template<typename T> struct std::iterator_traits<stripiter<T>> : std::iterator_traits<T> { };

constexpr auto
strip_begin(IterableCollection auto & cont)
{
	return stripiter {cont.begin() + 2};
}

constexpr auto
strip_end(IterableCollection auto & cont)
{
	return stripiter {cont.end()};
}

template<typename T, typename Dist, typename Merger>
void
mergeClose(std::vector<T> & range, const Dist & dist, const Merger & merger,
		decltype(dist(range.front(), range.front())) tolerance)
{
	using DistanceType = decltype(tolerance);
	std::vector<DistanceType> distances;
	distances.reserve(range.size() - 1);
	std::ranges::transform(range | std::views::pairwise, std::back_inserter(distances), [&dist](const auto & pair) {
		return (std::apply(dist, pair));
	});
	while (distances.size() > 1) {
		const auto closestPair = std::ranges::min_element(distances);
		if (*closestPair > tolerance) {
			return;
		}
		const auto offset = std::distance(distances.begin(), closestPair);
		const auto idx = static_cast<std::size_t>(offset);
		if (closestPair == distances.begin()) {
			// Remove second element
			range.erase(range.begin() + 1);
			distances.erase(distances.begin());
		}
		else if (closestPair == --distances.end()) {
			// Remove second from last element
			range.erase(range.end() - 2);
			distances.erase(distances.end() - 1);
		}
		else {
			range[idx] = merger(range[idx], range[idx + 1]);
			range.erase(range.begin() + offset + 1);
			distances.erase(distances.begin() + offset);
		}
		distances[idx] = dist(range[idx], range[idx + 1]);
	}
}
