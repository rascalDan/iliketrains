#pragma once

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>

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
