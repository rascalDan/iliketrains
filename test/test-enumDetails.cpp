#define BOOST_TEST_MODULE test_enumDetails

#include "enumDetailsData.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <enumDetails.hpp>
#include <stream_support.hpp>

constexpr std::array INVALID_NAMES {"", "missing", "GlobalScoped::aa", "GlobalScoped", "ns::aa", "a", "bb"};
constexpr std::array VALID_NAMES {"aa", "b", "c"};
template<typename E> constexpr std::array VALID_VALUES {E::aa, E::b, E::c};
// Not a template, else Boost test framework throws printing the context
constexpr std::array INVALID_VALUES {-1, 3, 20};

#define TESTS_FOR_TYPE(TYPE) \
	BOOST_DATA_TEST_CASE(invalid_check_##TYPE, INVALID_VALUES, in) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::is_valid(static_cast<TYPE>(in))); \
	} \
	BOOST_DATA_TEST_CASE(invalid_parse_##TYPE, INVALID_NAMES, in) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::parse(in).has_value()); \
	} \
	BOOST_DATA_TEST_CASE(invalid_to_string_##TYPE, INVALID_VALUES, in) \
	{ \
		BOOST_CHECK(!EnumDetails<TYPE>::to_string(static_cast<TYPE>(in)).has_value()); \
	} \
	BOOST_DATA_TEST_CASE(valid_check_##TYPE, VALID_VALUES<TYPE>, in) \
	{ \
		BOOST_CHECK(EnumDetails<TYPE>::is_valid(in)); \
	} \
	BOOST_DATA_TEST_CASE(valid_parse_##TYPE, VALID_NAMES ^ VALID_VALUES<TYPE>, in, out) \
	{ \
		const auto v = EnumDetails<TYPE>::parse(in); \
		BOOST_REQUIRE(v.has_value()); \
		BOOST_CHECK_EQUAL(v.value(), out); \
	} \
	BOOST_DATA_TEST_CASE(valid_to_string_##TYPE, VALID_VALUES<TYPE> ^ VALID_NAMES, in, out) \
	{ \
		const auto v = EnumDetails<TYPE>::to_string(in); \
		BOOST_CHECK(v.has_value()); \
		BOOST_CHECK_EQUAL(v.value(), out); \
	}

TESTS_FOR_TYPE(GlobalScoped)
TESTS_FOR_TYPE(GlobalUnscoped)
using ns_unscoped = ns::Unscoped;
using ns_scoped = ns::Scoped;
TESTS_FOR_TYPE(ns_unscoped)
TESTS_FOR_TYPE(ns_scoped)
