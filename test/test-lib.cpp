#define BOOST_TEST_MODULE test_lib

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include <collections.h>
#include <gfx/aabb.h>
#include <glArrays.h>
#include <glad/gl.h>
#include <set>

std::set<GLuint> active;

void
generator(GLsizei n, GLuint * out)
{
	static GLuint next {1};

	while (n--) {
		active.insert(next);
		*out++ = next++;
	}
}

void
deleter(GLsizei n, GLuint * in)
{
	BOOST_CHECK(std::all_of(in, in + n, [](GLuint id) {
		return active.erase(id) > 0;
	}));
}

using TestArray = glArrays<5, &generator, &deleter>;

BOOST_AUTO_TEST_CASE(generate_and_delete)
{
	{
		const TestArray a;
	}
	BOOST_CHECK(active.empty());
}

BOOST_AUTO_TEST_CASE(generate_move_and_delete)
{
	{
		TestArray a;
		BOOST_CHECK_EQUAL(TestArray::size(), active.size());
		const TestArray b {std::move(a)};
		BOOST_CHECK_EQUAL(TestArray::size(), active.size());
	}
	BOOST_CHECK(active.empty());
}

constexpr std::array TRIANGLE_STRIP_IN {0, 1, 2, 3, 4, 5};
static_assert(std::distance(strip_begin(TRIANGLE_STRIP_IN), strip_end(TRIANGLE_STRIP_IN)) == 4);

BOOST_AUTO_TEST_CASE(triangle_strip_iter)
{
	constexpr std::array TRIANGLE_STRIP_EXPECTED {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5};

	std::vector<int> out;
	std::for_each(strip_begin(TRIANGLE_STRIP_IN), strip_end(TRIANGLE_STRIP_IN), [&out](const auto & t) {
		const auto [a, b, c] = t;
		out.push_back(a);
		out.push_back(b);
		out.push_back(c);
	});
	BOOST_REQUIRE_EQUAL(out.size(), (TRIANGLE_STRIP_IN.size() - 2) * 3);
	BOOST_CHECK_EQUAL_COLCOL(out, TRIANGLE_STRIP_EXPECTED);
}

BOOST_AUTO_TEST_CASE(triangle_strip_range_adapter)
{
	using TriTuple = std::tuple<int, int, int>;
	std::vector<TriTuple> outRange;
	std::ranges::copy(TRIANGLE_STRIP_IN | triangleTriples, std::back_inserter(outRange));
	constexpr std::array<TriTuple, 4> TRIANGLE_STRIP_EXPECTED_TUPLES {{{0, 1, 2}, {2, 1, 3}, {2, 3, 4}, {4, 3, 5}}};
	BOOST_CHECK_EQUAL_COLCOL(outRange, TRIANGLE_STRIP_EXPECTED_TUPLES);
}

using MergeCloseData = std::tuple<std::vector<int>, int, std::vector<int>>;

BOOST_DATA_TEST_CASE(mergeCloseInts,
		boost::unit_test::data::make<MergeCloseData>({
				{{0}, 0, {0}},
				{{0, 1}, 0, {0, 1}},
				{{0, 1}, 2, {0, 1}},
				{{0, 1, 2}, 2, {0, 2}},
				{{0, 1, 4}, 2, {0, 4}},
				{{0, 1, 2}, 4, {0, 2}},
				{{0, 4, 8}, 4, {0, 8}},
				{{0, 4, 10, 14}, 4, {0, 14}},
				{{0, 3, 6}, 2, {0, 3, 6}},
				{{0, 3, 4}, 2, {0, 4}},
				{{0, 5, 7, 12}, 4, {0, 6, 12}},
				{{0, 3, 4, 5, 10, 17, 18, 19}, 2, {0, 4, 10, 19}},
		}),
		collection, tolerance, expected)
{
	auto mutableCollection {collection};
	BOOST_REQUIRE_NO_THROW((mergeClose(
			mutableCollection,
			[](int left, int right) {
				return std::abs(left - right);
			},
			[](int left, int right) {
				return (left + right) / 2;
			},
			tolerance)));
	BOOST_CHECK_EQUAL_COLCOL(mutableCollection, expected);
}

BOOST_AUTO_TEST_CASE(aabb_from_points)
{
	const auto aabb = AxisAlignedBoundingBox<GlobalDistance>::fromPoints(std::vector<GlobalPosition3D> {
			{1, 2, 3},
			{4, 2, 1},
			{9, 1, 7},
	});
	BOOST_CHECK_EQUAL(aabb.min, GlobalPosition3D(1, 1, 1));
	BOOST_CHECK_EQUAL(aabb.max, GlobalPosition3D(9, 2, 7));
}
