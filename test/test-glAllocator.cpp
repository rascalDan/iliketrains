#define BOOST_TEST_MODULE glAllocator

#include "testMainWindow.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "glAllocator.h"

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

namespace {
	BOOST_AUTO_TEST_CASE(Simple)
	{
		GLuint name = 0;
		{
			glVector<long double> list;
			BOOST_REQUIRE_EQUAL(list.get_allocator().getNameFor(list), 0);
			list.reserve(5);
			name = list.get_allocator().getNameFor(list);
			BOOST_REQUIRE_GT(name, 0);
			std::ranges::copy(std::views::iota(0, 10), std::back_inserter(list));
			BOOST_REQUIRE_EQUAL(10, list.size());
			BOOST_CHECK_EQUAL(0, list.front());
			BOOST_CHECK_EQUAL(9, list.back());
		}
		BOOST_CHECK(!glIsBuffer(name));
	}
}
