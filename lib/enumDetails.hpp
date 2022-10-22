#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string_view>

/// EnumTypeDetails
// Extracts the fully qualified name of the enumeration
template<typename E> struct EnumTypeDetails {
#ifndef ENUM_PROBE
protected:
#endif
	constexpr static std::string_view SEARCH_TYPE {"E = "};
	constexpr static auto
	typeraw()
	{
		return std::string_view {__PRETTY_FUNCTION__};
	};
	constexpr static auto typeNameStart {typeraw().find(SEARCH_TYPE) + SEARCH_TYPE.length()};
	constexpr static auto typeNameEnd {typeraw().find_first_of("];", typeNameStart)};
	constexpr static auto typeNameLen {typeNameEnd - typeNameStart};
	constexpr static auto typeNameArr {[]() {
		std::array<char, typeNameLen> out;
		typeraw().copy(out.begin(), typeNameEnd - typeNameStart, typeNameStart);
		return out;
	}()};

public:
	constexpr static std::string_view typeName {typeNameArr.data(), typeNameArr.size()};
};

/// EnumValueDetails
// Extracts the value name and constructs string_views of the parts
template<auto value> struct EnumValueDetails : public EnumTypeDetails<decltype(value)> {
#ifndef ENUM_PROBE
private:
#endif
	using T = EnumTypeDetails<decltype(value)>;
	constexpr static auto
	raw()
	{
		return std::string_view {__PRETTY_FUNCTION__};
	};
	constexpr static auto nameStart {raw().find_last_of(": ") + 1};
	constexpr static auto nameEnd {raw().find_first_of("];", nameStart)};
	constexpr static auto nameLen {nameEnd - nameStart};
	constexpr static auto nameArr {[]() {
		std::array<char, nameLen> out;
		raw().copy(out.begin(), nameLen, nameStart);
		return out;
	}()};

public:
	constexpr static std::string_view valueName {nameArr.data(), nameArr.size()};
	constexpr static auto valid {valueName.back() < '0' || valueName.back() > '9'};
	constexpr static auto raw_value {value};
};

template<typename E> struct EnumValueCollection {
	using Vs = std::make_integer_sequence<int, 256>;
};

template<typename E> struct EnumDetails {
public:
	using EVC = EnumValueCollection<E>;

#ifndef ENUM_PROBE
private:
#endif
	template<auto... n>
	constexpr static auto
	get_valids(std::integer_sequence<int, n...>)
	{
		return std::array {EnumValueDetails<static_cast<E>(n)>::valid...};
	}
	template<auto... n>
	constexpr static auto
	get_values(std::integer_sequence<int, n...>)
	{
		return std::array {EnumValueDetails<static_cast<E>(n)>::raw_value...};
	}
	template<auto... n>
	constexpr static auto
	get_valueNames(std::integer_sequence<int, n...>)
	{
		return std::array {EnumValueDetails<values[n]>::valueName...};
	}

	constexpr static auto valid_flags {get_valids(typename EVC::Vs {})};
	constexpr static auto valid_count {std::count_if(valid_flags.begin(), valid_flags.end(), std::identity {})};

public:
	constexpr static auto values {[]() {
		constexpr auto values {get_values(typename EVC::Vs {})};
		static_assert(std::is_sorted(values.begin(), values.end()), "Candidate values must be sorted");
		std::array<E, valid_count> out;
		auto write = out.begin();
		for (auto v = values.begin(); const bool & valid : valid_flags) {
			if (valid) {
				*write++ = static_cast<E>(*v);
			}
			++v;
		}
		return out;
	}()};
	constexpr static auto names {get_valueNames(std::make_integer_sequence<int, valid_count> {})};

	constexpr static bool
	is_valid(E value) noexcept
	{
		return std::binary_search(values.begin(), values.end(), value);
	}

	constexpr static std::optional<E>
	parse(std::string_view name) noexcept
	{
		if (const auto itr = std::find(names.begin(), names.end(), name); itr != names.end()) {
			return values[std::distance(names.begin(), itr)];
		}
		return std::nullopt;
	}

	constexpr static std::optional<std::string_view>
	to_string(E value) noexcept
	{
		if (const auto itr = std::find(values.begin(), values.end(), value); itr != values.end()) {
			return names[std::distance(values.begin(), itr)];
		}
		return std::nullopt;
	}
};
