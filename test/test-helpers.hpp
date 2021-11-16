#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>

#define BOOST_CHECK_CLOSE_VEC(a, b) \
	BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << a << ", " << b << ")") { \
		BOOST_CHECK_LT(glm::length(a - b), 0.1F); \
	}

#define BOOST_CHECK_BETWEEN(a, b, c) \
	BOOST_TEST_CONTEXT("BOOST_CHECK_BETWEEN(" << a << ", " << b << ", " << c << ")") { \
		BOOST_CHECK_LE(b, a); \
		BOOST_CHECK_GE(c, a); \
	}

#endif
