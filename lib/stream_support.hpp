#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <maths.h>
#include <span>
#include <sstream>

namespace std {
	template<typename T, std::size_t L>
	std::ostream &
	operator<<(std::ostream & s, const span<const T, L> v)
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

	template<typename T, std::size_t L>
	std::ostream &
	operator<<(std::ostream & s, const array<T, L> & v)
	{
		return (s << std::span {v});
	}

	template<typename T>
	std::ostream &
	operator<<(std::ostream & s, const vector<T> & v)
	{
		return (s << std::span {v});
	}

	inline std::ostream &
	operator<<(std::ostream & s, const Arc & arc)
	{
		return s << arc.first << " ↺ " << arc.second;
	}
}

template<typename T>
std::string
streamed_string(const T & v)
{
	std::stringstream ss;
	ss << v;
	return ss.str();
}

#define CLOG(x) std::cerr << #x " : " << x << "\n";
