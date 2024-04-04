#pragma once

#include "enumDetails.h"
#include <glm/glm.hpp>
#include <iostream>
#include <maths.h>
#include <span>
#include <sstream>
#include <type_traits>

template<typename S>
concept stringlike = requires(const S & s) { s.substr(0); };
template<typename T>
concept NonStringIterableCollection
		= std::is_same_v<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())> && !stringlike<T>;

namespace std {
	std::ostream &
	operator<<(std::ostream & s, const NonStringIterableCollection auto & v)
	{
		s << '(';
		for (const auto & i : v) {
			if (&i != &*v.begin()) {
				s << ", ";
			}
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

#define CLOG(x) std::cerr << __LINE__ << " : " #x " : " << x << "\n";
