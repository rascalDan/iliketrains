#pragma once

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip> // IWYU pragma: keep std::setprecision
#include <jsonParse-persistence.h>
#include <memory>

std::unique_ptr<char, decltype(&free)> uasprintf(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

template<typename T>
decltype(auto)
loadFixtureJson(const std::filesystem::path & path)
{
	std::ifstream in {FIXTURESDIR / path};
	return Persistence::JsonParsePersistence {}.loadState<std::vector<T>>(in);
}

#define BOOST_CHECK_CLOSE_VEC(a_, b_) \
	{ \
		const auto a {a_}, b {b_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << std::setprecision(8) << a << ", " << b << ")") { \
			BOOST_CHECK_LT(glm::length(a - b), 0.1F); \
		} \
	}

#define BOOST_CHECK_CLOSE_VECI(a_, b_) \
	{ \
		const auto a {a_}, b {b_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << std::setprecision(8) << a << ", " << b << ")") { \
			BOOST_CHECK_LE(std::abs(a.x - b.x), 1); \
			BOOST_CHECK_LE(std::abs(a.y - b.y), 1); \
			BOOST_CHECK_LE(std::abs(a.z - b.z), 1); \
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
