#define BOOST_TEST_MODULE pack

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "pack.h"
#include <vector>

using IntegerVectorPack = pack<int, std::vector>;

BOOST_FIXTURE_TEST_SUITE(pint, IntegerVectorPack)

BOOST_AUTO_TEST_CASE(basics)
{
	BOOST_CHECK_EQUAL(size(), 0);
	BOOST_CHECK_NO_THROW(emplace(1));
	BOOST_CHECK_NO_THROW(emplace(2));
	BOOST_CHECK_NO_THROW(emplace(3));
	BOOST_CHECK_NO_THROW(emplace(4));
	BOOST_CHECK_EQUAL(size(), 4);
	{
		std::array expected1 {1, 2, 3, 4};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}

	BOOST_CHECK_NO_THROW(erase(begin() + 1));
	BOOST_CHECK_EQUAL(size(), 3);
	{
		std::array expected1 {1, 4, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}

	BOOST_CHECK_NO_THROW(erase(--end()));
	BOOST_CHECK_EQUAL(size(), 2);
	{
		std::array expected1 {1, 4};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
}

BOOST_AUTO_TEST_SUITE_END()
