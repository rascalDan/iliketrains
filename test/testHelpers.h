#pragma once

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip> // IWYU pragma: keep std::setprecision
#include <jsonParse-persistence.h>

template<typename T>
decltype(auto)
loadFixtureJson(const std::filesystem::path & path)
{
	std::ifstream inputStream {FIXTURESDIR / path};
	return Persistence::JsonParsePersistence {}.loadState<std::vector<T>>(inputStream);
}

extern const std::filesystem::path ANALYSIS_DIRECTORY;

#define BOOST_CHECK_CLOSE_VEC(a_, b_) \
	{ \
		const auto left_ {a_}, right_ {b_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << std::setprecision(8) << left_ << ", " << right_ << ")") { \
			BOOST_CHECK_LT(glm::length(left_ - right_), 0.1F); \
		} \
	}

#define BOOST_CHECK_CLOSE_VECI(a_, b_) \
	{ \
		const auto left_ {a_}, right_ {b_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_CLOSE_VEC(" << std::setprecision(8) << left_ << ", " << right_ << ")") { \
			BOOST_CHECK_LE(std::abs(left_.x - right_.x), 1); \
			BOOST_CHECK_LE(std::abs(left_.y - right_.y), 1); \
			BOOST_CHECK_LE(std::abs(left_.z - right_.z), 1); \
		} \
	}

#define BOOST_CHECK_BETWEEN(a_, b_, c_) \
	{ \
		const auto value_ {a_}, min_ {b_}, max_ {c_}; \
		BOOST_TEST_CONTEXT("BOOST_CHECK_BETWEEN(" << min_ << " <= " << value_ << " <= " << max_ << ")") { \
			BOOST_CHECK_LE(min_, value_); \
			BOOST_CHECK_GE(max_, value_); \
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

#define BOOST_CHECK_EQUAL_COLCOL(cola_, colb_) \
	BOOST_CHECK_EQUAL_COLLECTIONS(cola_.begin(), cola_.end(), colb_.begin(), colb_.end())
#define BOOST_CHECK_EQUAL_COLVALS(col_, ...) \
	{ \
		const std::initializer_list<decltype(col_)::value_type> vals {__VA_ARGS__}; \
		BOOST_CHECK_EQUAL_COLLECTIONS(col_.begin(), col_.end(), vals.begin(), vals.end()); \
	}
