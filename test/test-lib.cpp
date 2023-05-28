#define BOOST_TEST_MODULE test_lib

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

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
