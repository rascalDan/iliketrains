#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string_view>

/// EnumDetailsBase
// Shared helpers
struct EnumDetailsBase {
	template<size_t len>
	constexpr static auto
	strArr(auto input, auto start, auto end)
	{
		std::array<char, len> out;
		input.copy(out.begin(), end - start, start);
		return out;
	}
};

/// EnumTypeDetails
// Extracts the fully qualified name of the enumeration
template<typename E> struct EnumTypeDetails : EnumDetailsBase {
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
	constexpr static auto typeNameArr {strArr<typeNameLen>(typeraw(), typeNameStart, typeNameEnd)};

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
	constexpr static auto nameArr {EnumValueDetails::template strArr<nameLen>(raw(), nameStart, nameEnd)};

public:
	constexpr static std::string_view valueName {nameArr.data(), nameArr.size()};
	constexpr static auto valid {valueName.back() < '0' || valueName.back() > '9'};
	constexpr static auto raw_value {value};
};

/// EnumValueCollection
// Customisation point for specifying the range of underlying values your enum can have
template<typename E> struct EnumValueCollection {
	using Vs = std::make_integer_sequence<int, 256>;
};

/// EnumDetails
// Interface for lookups/checks/etc at runtime
template<typename E> struct EnumDetails {
#ifndef ENUM_PROBE
private:
#endif
	template<auto... n>
	constexpr static auto
	get_valids(std::integer_sequence<int, n...>)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
		return std::array {EnumValueDetails<static_cast<E>(n)>::valid...};
#pragma GCC diagnostic pop
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

	using EVC = EnumValueCollection<E>;
	constexpr static auto valid_flags {get_valids(typename EVC::Vs {})};
	constexpr static auto valid_count {std::count_if(valid_flags.begin(), valid_flags.end(), std::identity {})};

	constexpr static auto
	lookup(const auto key, const auto & search,
			const auto & out) -> std::optional<typename std::decay_t<decltype(out)>::value_type>
	{
		if (const auto itr = std::find(search.begin(), search.end(), key); itr != search.end()) {
			return out[static_cast<std::size_t>(std::distance(search.begin(), itr))];
		}
		return std::nullopt;
	}

public:
	constexpr static auto values {[]() {
		constexpr auto values {get_values(typename EVC::Vs {})};
		static_assert(std::is_sorted(values.begin(), values.end()), "Candidate values must be sorted");
		std::array<E, valid_count> out;
		std::copy_if(values.begin(), values.end(), out.begin(), [valid = valid_flags.begin()](auto) mutable {
			return *valid++;
		});
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
		return lookup(name, names, values);
	}

	constexpr static std::optional<std::string_view>
	to_string(E value) noexcept
	{
		return lookup(value, values, names);
	}
};
