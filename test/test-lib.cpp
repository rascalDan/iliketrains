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

namespace {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) - tracker
	std::set<GLuint> active;

	void
	generator(GLsizei count, GLuint * out)
	{
		static GLuint next {1};

		std::generate_n(out, count, []() {
			return *active.emplace(next++).first;
		});
	}

	void
	deleter(GLsizei n, GLuint * input)
	{
		BOOST_CHECK(std::ranges::all_of(std::span(input, static_cast<size_t>(n)), [](GLuint name) {
			return active.erase(name) > 0;
		}));
	}

	using TestArray = glArrays<5, &generator, &deleter>;
}

BOOST_AUTO_TEST_CASE(GenerateAndDelete)
{
	{
		const TestArray arr;
	}
	BOOST_CHECK(active.empty());
}

BOOST_AUTO_TEST_CASE(GenerateMoveAndDelete)
{
	{
		TestArray arr1;
		BOOST_CHECK_EQUAL(TestArray::size(), active.size());
		const TestArray arr2 {std::move(arr1)};
		BOOST_CHECK_EQUAL(TestArray::size(), active.size());
	}
	BOOST_CHECK(active.empty());
}

constexpr std::array TRIANGLE_STRIP_IN {0, 1, 2, 3, 4, 5};
static_assert(std::distance(strip_begin(TRIANGLE_STRIP_IN), strip_end(TRIANGLE_STRIP_IN)) == 4);

BOOST_AUTO_TEST_CASE(TriangleStripIter)
{
	constexpr std::array TRIANGLE_STRIP_EXPECTED {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5};

	std::vector<int> out;
	std::for_each(strip_begin(TRIANGLE_STRIP_IN), strip_end(TRIANGLE_STRIP_IN), [&out](const auto & triangle) {
		const auto [corner1, corner2, corner3] = triangle;
		out.push_back(corner1);
		out.push_back(corner2);
		out.push_back(corner3);
	});
	BOOST_REQUIRE_EQUAL(out.size(), (TRIANGLE_STRIP_IN.size() - 2) * 3);
	BOOST_CHECK_EQUAL_COLCOL(out, TRIANGLE_STRIP_EXPECTED);
}

BOOST_AUTO_TEST_CASE(TriangleStripRangeAdapter)
{
	using TriTuple = std::tuple<int, int, int>;
	std::vector<TriTuple> outRange;
	std::ranges::copy(TRIANGLE_STRIP_IN | triangleTriples, std::back_inserter(outRange));
	constexpr std::array<TriTuple, 4> TRIANGLE_STRIP_EXPECTED_TUPLES {{{0, 1, 2}, {2, 1, 3}, {2, 3, 4}, {4, 3, 5}}};
	BOOST_CHECK_EQUAL_COLCOL(outRange, TRIANGLE_STRIP_EXPECTED_TUPLES);
}

using MergeCloseData = std::tuple<std::vector<int>, int, std::vector<int>>;

BOOST_DATA_TEST_CASE(MergeCloseInts,
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

BOOST_AUTO_TEST_CASE(AABBFromPoints)
{
	const auto aabb = AxisAlignedBoundingBox<GlobalDistance>::fromPoints(std::vector<GlobalPosition3D> {
			{1, 2, 3},
			{4, 2, 1},
			{9, 1, 7},
	});
	BOOST_CHECK_EQUAL(aabb.min, GlobalPosition3D(1, 1, 1));
	BOOST_CHECK_EQUAL(aabb.max, GlobalPosition3D(9, 2, 7));
}
