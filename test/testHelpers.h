#pragma once

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>
#include <memory>

std::unique_ptr<char, decltype(&free)> uasprintf(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

#define BOOST_CHECK_CLOSE_VEC(a_, b_) \
	{ \
		const auto a {a_}, b {b_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << a << ", " << b << ")") { \
			BOOST_CHECK_LT(glm::length(a - b), 0.1F); \
		} \
	}

#define BOOST_CHECK_BETWEEN(a_, b_, c_) \
	{ \
		const auto a {a_}, b {b_}, c {c_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_BETWEEN(" << a << ", " << b << ", " << c << ")") { \
			BOOST_CHECK_LE(b, a); \
			BOOST_CHECK_GE(c, a); \
		} \
	}
#define BOOST_REQUIRE_THEN(VAR, EXPR) \
	if (const auto VAR = (EXPR); !(VAR)) { \
		BOOST_REQUIRE(VAR); \
	} \
	else
#define BOOST_CHECK_IF(VAR, EXPR) \
	if (const auto VAR = (EXPR); !(VAR)) { \
		BOOST_CHECK(VAR); \
	} \
	else
