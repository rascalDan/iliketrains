#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <tuple>

template<typename T, std::size_t N>
constexpr auto
transform_array(const std::array<T, N> & in, auto && transform)
{
	std::array<decltype(transform(in.front())), N> out {};
	std::transform(in.begin(), in.end(), out.begin(), transform);
	return out;
}

namespace {
	template<size_t... N> struct GetNth {
		decltype(auto)
		operator()(const auto & tup) const
		{
			if constexpr (sizeof...(N) == 1) {
				return std::get<N...>(tup);
			}
			else {
				return std::tie(std::get<N>(tup)...);
			}
		}
	};
}

template<size_t... N> constexpr auto Nth = GetNth<N...> {};
constexpr auto GetFirst = Nth<0>;
constexpr auto GetSecond = Nth<1>;
constexpr auto GetSwapped = Nth<0, 1>;
