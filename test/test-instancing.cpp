#define BOOST_TEST_MODULE instancing

#include "testHelpers.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <gfx/gl/instanceVertices.h>

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

BOOST_FIXTURE_TEST_SUITE(i, InstanceVertices<int>)

BOOST_AUTO_TEST_CASE(createDestroy)
{
	BOOST_CHECK(!data);
	map();
	BOOST_REQUIRE(data);
	BOOST_CHECK_EQUAL(0, next);
	BOOST_CHECK(unused.empty());
	unmap();
	BOOST_CHECK(!data);
}

BOOST_AUTO_TEST_CASE(storeRetreive)
{ // Read write raw buffer, not normally allowed
	std::vector<int> test(capacity);
	map();
	std::copy(test.begin(), test.end(), data);
	BOOST_CHECK_EQUAL_COLLECTIONS(test.begin(), test.end(), data, data + capacity);
}

BOOST_AUTO_TEST_CASE(acquireRelease)
{
	{
		auto proxy = acquire();
		*proxy = 20;
		BOOST_CHECK_EQUAL(1, next);
	}
	BOOST_CHECK_EQUAL(1, next);
	BOOST_CHECK_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(0, unused.front());
}

BOOST_AUTO_TEST_CASE(acquireReleaseMove)
{
	{
		auto proxy1 = acquire();
		*proxy1 = 20;
		BOOST_CHECK_EQUAL(1, next);
		auto proxy2 = std::move(proxy1);
		*proxy2 = 40;
		BOOST_CHECK_EQUAL(1, next);
	}
	BOOST_CHECK_EQUAL(1, next);
	BOOST_CHECK_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(0, unused.front());
}

BOOST_AUTO_TEST_CASE(initialize)
{
	auto proxy = acquire(5);
	BOOST_CHECK_EQUAL(*proxy, 5);
}

BOOST_AUTO_TEST_CASE(resize)
{
	constexpr auto COUNT = 500;
	std::vector<decltype(acquire())> proxies;
	std::vector<int> expected;
	for (auto n = 0; n < COUNT; n++) {
		proxies.push_back(acquire(n));
		expected.emplace_back(n);
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), data, data + COUNT);
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), proxies.begin(), proxies.end());
}

BOOST_AUTO_TEST_SUITE_END()
