#define BOOST_TEST_MODULE test_geo

#include "test-helpers.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.hpp>

#include <game/geoData.h>

struct TestGeoData : public GeoData {
	TestGeoData() : GeoData {{{-10, -5}, {30, 40}}, 5.F} { }
};

BOOST_FIXTURE_TEST_CASE(initialize, TestGeoData)
{
	BOOST_CHECK_EQUAL(limit.first, glm::ivec2(-10, -5));
	BOOST_CHECK_EQUAL(limit.second, glm::ivec2(30, 40));
	BOOST_CHECK_EQUAL(scale, 5.F);
	BOOST_CHECK_EQUAL(size, glm::uvec2(41, 46));
	BOOST_CHECK_EQUAL(nodes.size(), 1886);
	BOOST_CHECK(std::all_of(nodes.begin(), nodes.end(), [](const auto & n) {
		return n.height == -1.5F;
	}));
}

BOOST_FIXTURE_TEST_CASE(coords, TestGeoData)
{
	BOOST_CHECK_EQUAL(at(-10, -5), 0);
	BOOST_CHECK_EQUAL(at(-9, -5), 1);
	BOOST_CHECK_EQUAL(at(0, -5), 10);
	BOOST_CHECK_EQUAL(at(30, -5), 40);
	BOOST_CHECK_EQUAL(at(30, 40), 1885);
}

BOOST_FIXTURE_TEST_CASE(coords_bad, TestGeoData)
{
	BOOST_CHECK_THROW(std::ignore = at(-11, -5), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(-10, -6), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(-11, -6), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(31, 40), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(30, 41), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(31, 41), std::range_error);
}

BOOST_FIXTURE_TEST_CASE(gen_random, TestGeoData)
{
	// Can only really its sanity
	generateRandom();
	// Some terrain above sea level
	BOOST_CHECK(std::any_of(nodes.begin(), nodes.end(), [](const auto & n) {
		return n.height > 0;
	}));
	// Still an island
	for (int x = limit.first.x; x <= limit.second.x; x += 1) {
		BOOST_CHECK_EQUAL(nodes[at(x, limit.first.y)].height, -1.5F);
		BOOST_CHECK_EQUAL(nodes[at(x, limit.second.y)].height, -1.5F);
	}
	for (int y = limit.first.y; y <= limit.second.y; y += 1) {
		BOOST_CHECK_EQUAL(nodes[at(limit.first.x, y)].height, -1.5F);
		BOOST_CHECK_EQUAL(nodes[at(limit.second.x, y)].height, -1.5F);
	}
}

BOOST_FIXTURE_TEST_CASE(load_uk_heightmap, TestGeoData)
{
	loadFromImages(FIXTURESDIR "/height/V0txo.jpg", 100.F);
	// Some terrain above sea level
	BOOST_CHECK(std::any_of(nodes.begin(), nodes.end(), [](const auto & n) {
		return n.height > 0;
	}));
}