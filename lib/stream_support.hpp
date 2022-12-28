#pragma once

#include "enumDetails.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <maths.h>
#include <span>
#include <sstream>
#include <type_traits>

template<typename S>
concept stringlike = requires(const S & s) { s.substr(0); };
template<typename T>
concept spanable = std::is_constructible_v<std::span<const typename T::value_type>, T> && !
stringlike<T> && !std::is_same_v<std::span<typename T::value_type>, T>;

namespace std {
	template<typename T, std::size_t L>
	std::ostream &
	operator<<(std::ostream & s, const span<T, L> v)
	{
		s << '(';
		for (const auto & i : v) {
			if (&i != &v.front())
				s << ", ";
			s << i;
		}
		return s << ')';
	}

	template<glm::length_t L, glm::length_t R, typename T, glm::qualifier Q>
	std::ostream &
	operator<<(std::ostream & s, const glm::mat<L, R, T, Q> & m)
	{
		return (s << std::span {&m[0], L});
	}

	template<glm::length_t L, typename T, glm::qualifier Q>
	std::ostream &
	operator<<(std::ostream & s, const glm::vec<L, T, Q> & v)
	{
		return (s << std::span {&v[0], L});
	}

	template<spanable T>
	std::ostream &
	operator<<(std::ostream & s, const T & v)
	{
		return (s << std::span {v});
	}

	template<typename First, typename Second>
	std::ostream &
	operator<<(std::ostream & s, const std::pair<First, Second> & v)
	{
		return (s << '(' << v.first << ", " << v.second << ')');
	}

	inline std::ostream &
	operator<<(std::ostream & s, const Arc & arc)
	{
		return s << arc.first << " ↺ " << arc.second;
	}

	template<typename E>
	concept IsEnum = std::is_enum_v<E>;

	template<IsEnum E>
	inline std::ostream &
	operator<<(std::ostream & s, const E & e)
	{
		return s << EnumTypeDetails<E>::typeName << "::" << EnumDetails<E>::to_string(e).value();
	}
}

template<typename T>
std::string
streamed_string(const T & v)
{
	std::stringstream ss;
	ss << v;
	return std::move(ss).str();
}

#define CLOG(x) std::cerr << #x " : " << x << "\n";
