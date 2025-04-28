#pragma once

#include "enumDetails.h"
#include <glm/glm.hpp>
#include <iostream>
#include <maths.h>
#include <optional>
#include <source_location>
#include <span>
#include <sstream>
#include <tuple>
#include <type_traits>

template<typename S>
concept stringlike = requires(const S & s) { s.substr(0); };
template<typename T>
concept NonStringIterableCollection
		= std::is_same_v<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())> && !stringlike<T>;

namespace std {
	std::ostream &
	operator<<(std::ostream & s, const NonStringIterableCollection auto & collection)
	{
		s << '(';
		for (size_t nth {}; const auto & element : collection) {
			if (nth++) {
				s << ", ";
			}
			s << element;
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

	namespace {
		template<typename... T, size_t... Idx>
		std::ostream &
		printTuple(std::ostream & s, const std::tuple<T...> & v, std::integer_sequence<size_t, Idx...>)
		{
			return ((s << (Idx ? ", " : "") << std::get<Idx>(v)), ...);
		}
	}

	template<typename... T>
	std::ostream &
	operator<<(std::ostream & s, const std::tuple<T...> & v)
	{
		return printTuple(s << '{', v, std::make_index_sequence<sizeof...(T)>()) << '}';
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
		return s << EnumTypeDetails<E>::TYPE_NAME << "::" << EnumDetails<E>::toString(e).value();
	}

	template<typename T>
	inline std::ostream &
	operator<<(std::ostream & s, const std::optional<T> & v)
	{
		if (v) {
			return s << *v;
		}
		return s << "nullopt";
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

namespace {
	template<typename T>
	void
	clogImpl(const T & value, const std::string_view name,
			const std::source_location loc = std::source_location::current())
	{
		std::cerr << loc.line() << " : " << name << " : " << value << "\n";
	}
}

#define CLOG(x) clogImpl(x, #x)
#define CLOGf(...) clogImpl(std::format(__VA_ARGS__), "msg")
