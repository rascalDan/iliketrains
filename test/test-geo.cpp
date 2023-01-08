#define BOOST_TEST_MODULE test_geo

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.hpp>

#include <game/geoData.h>
#include <lib/ray.hpp>

struct TestGeoData : public GeoData {
	TestGeoData() : GeoData {{{-10, -5}, {30, 40}}, 5.F} { }
};

namespace std {
	std::ostream &
	operator<<(std::ostream & s, const Ray & r)
	{
		return (s << r.start << "->" << r.direction);
	}
}

BOOST_FIXTURE_TEST_SUITE(tgd, TestGeoData)

BOOST_AUTO_TEST_CASE(initialize)
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

BOOST_AUTO_TEST_CASE(coords)
{
	BOOST_CHECK_EQUAL(at(-10, -5), 0);
	BOOST_CHECK_EQUAL(at(-9, -5), 1);
	BOOST_CHECK_EQUAL(at(0, -5), 10);
	BOOST_CHECK_EQUAL(at(30, -5), 40);
	BOOST_CHECK_EQUAL(at(30, 40), 1885);
}

BOOST_AUTO_TEST_CASE(coords_bad)
{
	BOOST_CHECK_THROW(std::ignore = at(-11, -5), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(-10, -6), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(-11, -6), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(31, 40), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(30, 41), std::range_error);
	BOOST_CHECK_THROW(std::ignore = at(31, 41), std::range_error);
}

BOOST_AUTO_TEST_CASE(gen_random)
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

BOOST_AUTO_TEST_CASE(load_uk_heightmap)
{
	loadFromImages(FIXTURESDIR "/height/V0txo.jpg", 100.F);
	// Some terrain above sea level
	BOOST_CHECK(std::any_of(nodes.begin(), nodes.end(), [](const auto & n) {
		return n.height > 0;
	}));
}

BOOST_AUTO_TEST_CASE(get_height_at)
{
	// at(x,y) is index based
	nodes[at(0, 0)].height = 1;
	nodes[at(1, 0)].height = 2;
	nodes[at(0, 1)].height = 3;
	nodes[at(1, 1)].height = 4;

	// positionAt(x,y) is world based
	// Corners
	BOOST_CHECK_EQUAL(positionAt({0, 0}), glm::vec3(0, 0, 1));
	BOOST_CHECK_EQUAL(positionAt({5, 0}), glm::vec3(5, 0, 2));
	BOOST_CHECK_EQUAL(positionAt({0, 5}), glm::vec3(0, 5, 3));
	BOOST_CHECK_EQUAL(positionAt({5, 5}), glm::vec3(5, 5, 4));

	// Edge midpoints
	BOOST_CHECK_EQUAL(positionAt({0, 2.5F}), glm::vec3(0, 2.5F, 2));
	BOOST_CHECK_EQUAL(positionAt({5, 2.5F}), glm::vec3(5, 2.5F, 3));
	BOOST_CHECK_EQUAL(positionAt({2.5F, 0}), glm::vec3(2.5F, 0, 1.5F));
	BOOST_CHECK_EQUAL(positionAt({2.5F, 5}), glm::vec3(2.5F, 5, 3.5F));

	// Centre
	BOOST_CHECK_EQUAL(positionAt({2.5F, 2.5F}), glm::vec3(2.5F, 2.5F, 2.5F));
}

using TestRayTracerData = std::tuple<glm::vec2, glm::vec2, glm::vec2::value_type, std::vector<glm::vec2>>;
BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))
BOOST_DATA_TEST_CASE(raytracer,
		boost::unit_test::data::make<TestRayTracerData>({
				{{1, 2}, {4, 5}, 4,
						{
								{0, 0},
								{0, 4},
								{4, 4},
								{4, 8},
								{8, 8},
								{8, 12},
								{12, 12},
								{12, 16},
								{12, 20},
								{16, 20},
						}},
				{{-1, -1}, {-4, -5}, 5,
						{
								{-5, -5},
								{-5, -10},
								{-10, -10},
								{-10, -15},
								{-15, -15},
								{-15, -20},
								{-20, -20},
								{-20, -25},
						}},
		}),
		start, dir, scale, points)
{
	GeoData::RayTracer rt {start / scale, glm::normalize(dir)};
	for (const auto & point : points) {
		BOOST_CHECK_CLOSE_VEC(point, rt.next() * scale);
	}
}

using TestRayData = std::tuple<glm::vec3, glm::vec3, glm::vec3>;
BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))
BOOST_DATA_TEST_CASE(intersect_ray,
		boost::unit_test::data::make<TestRayData>({
				{{-1, -1, 1.0}, {1, 1, 0}, {0, 0, 1}},
				{{-1, -1, 2.5}, {1, 1, 0}, {2.5F, 2.5F, 2.5F}},
				{{-20, -20, 2.5}, {1, 1, 0}, {2.5F, 2.5F, 2.5F}},
				// outside the map looking in
				{{-205, -205, 4}, {1, 1, 0}, {5, 5, 4}},
				{{-205, 5, 4}, {1, 0, 0}, {5, 5, 4}},
				{{-205, 215, 4}, {1, -1, 0}, {5, 5, 4}},
				{{215, -205, 4}, {-1, 1, 0}, {5, 5, 4}},
				{{215, 5, 4}, {-1, 0, 0}, {5, 5, 4}},
				{{215, 215, 4}, {-1, -1, 0}, {5, 5, 4}},
				{{5, 215, 4}, {0, -1, 0}, {5, 5, 4}},
				{{5, -205, 4}, {0, 1, 0}, {5, 5, 4}},
		}),
		start, dir, pos)
{
	// at(x,y) is index based
	nodes[at(0, 0)].height = 1;
	nodes[at(1, 0)].height = 2;
	nodes[at(0, 1)].height = 3;
	nodes[at(1, 1)].height = 4;

	const auto intersect = intersectRay({start, glm::normalize(dir)});
	BOOST_CHECK_IF(has_intersect, intersect) {
		BOOST_CHECK_CLOSE_VEC(*intersect, pos);
	}
}

auto xs = boost::unit_test::data::xrange(-20.F, 0.F, 0.6F), ys = boost::unit_test::data::xrange(-20.F, 0.F, 0.7F);
auto targetsx = boost::unit_test::data::xrange(0.2F, 4.9F, 1.3F),
	 targetsy = boost::unit_test::data::xrange(0.3F, 4.9F, 1.3F);
BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))
BOOST_DATA_TEST_CASE(intersect_ray_many, xs * ys * targetsx * targetsy, x, y, targetx, targety)
{
	// at(x,y) is index based
	nodes[at(0, 0)].height = 1;
	nodes[at(1, 0)].height = 2;
	nodes[at(0, 1)].height = 3;
	nodes[at(1, 1)].height = 4;

	const glm::vec3 start {x, y, 10};
	const auto target {this->positionAt({targetx, targety})};
	const Ray ray {start, glm::normalize(target - start)};
	BOOST_TEST_CONTEXT(ray) {
		const auto intersect = intersectRay(ray);
		BOOST_CHECK_IF(has_intersect, intersect) {
			BOOST_CHECK_CLOSE_VEC(*intersect, target);
		}
	}
}

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))
BOOST_DATA_TEST_CASE(intersect_ray_miss,
		boost::unit_test::data::make<Ray>({
				{{3, 3, 5}, {-1, -1, 0}},
				{{0, 0, 5}, {0, 0, 1}},
				{{0, 0, 5}, {0, 0, -1}},
		}),
		ray)
{
	// at(x,y) is index based
	nodes[at(0, 0)].height = 1;
	nodes[at(1, 0)].height = 2;
	nodes[at(0, 1)].height = 3;
	nodes[at(1, 1)].height = 4;

	BOOST_REQUIRE(!intersectRay(ray));
}

BOOST_AUTO_TEST_SUITE_END()
