#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>

template<typename T, std::size_t N>
constexpr auto
transform_array(const std::array<T, N> & in, auto && transform)
{
	std::array<decltype(transform(in.front())), N> out {};
	std::transform(in.begin(), in.end(), out.begin(), transform);
	return out;
}
