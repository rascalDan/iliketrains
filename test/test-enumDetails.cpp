#define BOOST_TEST_MODULE test_enumDetails

#include "enumDetailsData.h"
#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <enumDetails.h>
#include <stream_support.h>

constexpr std::array INVALID_NAMES {"", "missing", "GlobalScoped::Aa", "GlobalScoped", "ns::Aa", "A", "Bb"};
constexpr std::array VALID_NAMES {"Aa", "B", "C"};
template<typename E> constexpr std::array VALID_VALUES {E::Aa, E::B, E::C};
// Not a template, else Boost test framework throws printing the context
constexpr std::array INVALID_VALUES {-1, 3, 20};

#define TESTS_FOR_TYPE(TYPE) \
	BOOST_DATA_TEST_CASE(invalid_check_##TYPE, INVALID_VALUES, input) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::isValid(static_cast<TYPE>(input))); \
	} \
	BOOST_DATA_TEST_CASE(invalid_parse_##TYPE, INVALID_NAMES, input) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::parse(input).has_value()); \
	} \
	BOOST_DATA_TEST_CASE(invalid_to_string_##TYPE, INVALID_VALUES, input) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::toString(static_cast<TYPE>(input)).has_value()); \
	} \
	BOOST_DATA_TEST_CASE(valid_check_##TYPE, VALID_VALUES<TYPE>, input) \
	{ \
		BOOST_CHECK(EnumDetails<TYPE>::isValid(input)); \
	} \
	BOOST_DATA_TEST_CASE(valid_parse_##TYPE, VALID_NAMES ^ VALID_VALUES<TYPE>, input, output) \
	{ \
		const auto parsed = EnumDetails<TYPE>::parse(input); \
		BOOST_CHECK_IF(parsedOK, parsed.has_value()) { \
			BOOST_CHECK_EQUAL(parsed.value(), output); \
		} \
	} \
	BOOST_DATA_TEST_CASE(valid_to_string_##TYPE, VALID_VALUES<TYPE> ^ VALID_NAMES, input, output) \
	{ \
		const auto parsed = EnumDetails<TYPE>::toString(input); \
		BOOST_CHECK_IF(parsedOK, parsed.has_value()) { \
			BOOST_CHECK_EQUAL(parsed.value(), output); \
		} \
	}

TESTS_FOR_TYPE(GlobalScoped)
TESTS_FOR_TYPE(GlobalUnscoped)
using NsUnscoped = ns::Unscoped;
using NsScoped = ns::Scoped;
TESTS_FOR_TYPE(NsUnscoped)
TESTS_FOR_TYPE(NsScoped)
