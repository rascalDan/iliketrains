#define BOOST_TEST_MODULE test_lib

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <collections.h>
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
		BOOST_CHECK_EQUAL(TestArray::size, active.size());
		const TestArray b {std::move(a)};
		BOOST_CHECK_EQUAL(TestArray::size, active.size());
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
	BOOST_CHECK_EQUAL_COLLECTIONS(
			out.begin(), out.end(), TRIANGLE_STRIP_EXPECTED.begin(), TRIANGLE_STRIP_EXPECTED.end());
}
