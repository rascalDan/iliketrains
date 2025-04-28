#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string_view>

/// EnumDetailsBase
// Shared helpers
struct EnumDetailsBase {
	template<size_t Len>
	constexpr static auto
	strArr(auto input, auto start, auto end)
	{
		std::array<char, Len> out;
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

	constexpr static auto TYPE_NAME_START {typeraw().find(SEARCH_TYPE) + SEARCH_TYPE.length()};
	constexpr static auto TYPE_NAME_END {typeraw().find_first_of("];", TYPE_NAME_START)};
	constexpr static auto TYPE_NAME_LEN {TYPE_NAME_END - TYPE_NAME_START};
	constexpr static auto TYPE_NAME_ARR {strArr<TYPE_NAME_LEN>(typeraw(), TYPE_NAME_START, TYPE_NAME_END)};

public:
	constexpr static std::string_view TYPE_NAME {TYPE_NAME_ARR.data(), TYPE_NAME_ARR.size()};
};

/// EnumValueDetails
// Extracts the value name and constructs string_views of the parts
template<auto Value> struct EnumValueDetails : public EnumTypeDetails<decltype(Value)> {
#ifndef ENUM_PROBE
private:
#endif
	using T = EnumTypeDetails<decltype(Value)>;

	constexpr static auto
	raw()
	{
		return std::string_view {__PRETTY_FUNCTION__};
	};

	constexpr static auto NAME_START {raw().find_last_of(": ") + 1};
	constexpr static auto NAME_END {raw().find_first_of("];", NAME_START)};
	constexpr static auto NAME_LEN {NAME_END - NAME_START};
	constexpr static auto NAME_ARR {EnumValueDetails::template strArr<NAME_LEN>(raw(), NAME_START, NAME_END)};

public:
	constexpr static std::string_view VALUE_NAME {NAME_ARR.data(), NAME_ARR.size()};
	constexpr static auto VALID {VALUE_NAME.back() < '0' || VALUE_NAME.back() > '9'};
	constexpr static auto RAW_VALUE {Value};
};

/// EnumValueCollection
// Customisation point for specifying the range of underlying values your enum can have
template<typename E> struct EnumValueCollection {
	using Underlying = std::underlying_type_t<E>;
	static constexpr Underlying UPPER = std::numeric_limits<Underlying>::max();
	using Vs = std::make_integer_sequence<Underlying, UPPER>;
};

/// EnumDetails
// Interface for lookups/checks/etc at runtime
template<typename E> struct EnumDetails {
#ifndef ENUM_PROBE
private:
#endif
	using Underlying = std::underlying_type_t<E>;

	template<auto... N>
	constexpr static auto
	getValids(std::integer_sequence<Underlying, N...>)
	{
		return std::array {EnumValueDetails<static_cast<E>(N)>::VALID...};
	}

	template<auto... N>
	constexpr static auto
	getValues(std::integer_sequence<Underlying, N...>)
	{
		return std::array {EnumValueDetails<static_cast<E>(N)>::RAW_VALUE...};
	}

	template<auto... N>
	constexpr static auto
	getValueNames(std::integer_sequence<int, N...>)
	{
		return std::array {EnumValueDetails<VALUES[N]>::VALUE_NAME...};
	}

	using EVC = EnumValueCollection<E>;
	constexpr static auto VALID_FLAGS {getValids(typename EVC::Vs {})};
	constexpr static auto VALID_COUNT {std::count_if(VALID_FLAGS.begin(), VALID_FLAGS.end(), std::identity {})};

	constexpr static auto
	lookup(const auto key, const auto & search, const auto & out)
			-> std::optional<typename std::decay_t<decltype(out)>::value_type>
	{
		if (const auto itr = std::find(search.begin(), search.end(), key); itr != search.end()) {
			return out[static_cast<std::size_t>(std::distance(search.begin(), itr))];
		}
		return std::nullopt;
	}

public:
	constexpr static auto VALUES {[]() {
		constexpr auto VALUES {getValues(typename EVC::Vs {})};
		static_assert(std::ranges::is_sorted(VALUES), "Candidate values must be sorted");
		std::array<E, VALID_COUNT> out;
		std::copy_if(VALUES.begin(), VALUES.end(), out.begin(), [valid = VALID_FLAGS.begin()](auto) mutable {
			return *valid++;
		});
		return out;
	}()};
	constexpr static auto NAMES {getValueNames(std::make_integer_sequence<int, VALID_COUNT> {})};

	constexpr static bool
	isValid(E value) noexcept
	{
		return std::binary_search(VALUES.begin(), VALUES.end(), value);
	}

	constexpr static std::optional<E>
	parse(std::string_view name) noexcept
	{
		return lookup(name, NAMES, VALUES);
	}

	constexpr static std::optional<std::string_view>
	toString(E value) noexcept
	{
		return lookup(value, VALUES, NAMES);
	}
};
