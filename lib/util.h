#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <memory>
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

template<size_t... N> inline constexpr auto Nth = GetNth<N...> {};
inline constexpr auto GetFirst = Nth<0>;
inline constexpr auto GetSecond = Nth<1>;
inline constexpr auto GetSwapped = Nth<0, 1>;

template<typename T, typename M> struct Decompose {
	consteval Decompose(M T::*) { }

	using ValueType = M;
	using ContainerType = T;
};

template<auto MbrPtr> using MemberValueType = typename decltype(Decompose {MbrPtr})::ValueType;
template<auto MbrPtr> using ContainerType = typename decltype(Decompose {MbrPtr})::ContainerType;

template<typename T>
bool
createIfRequired(std::shared_ptr<T> & instance, std::weak_ptr<T> & common)
{
	if (!instance && !(instance = common.lock())) {
		common = instance = std::make_shared<T>();
		return true;
	}
	return false;
}
