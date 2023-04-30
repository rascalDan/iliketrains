#define BOOST_TEST_MODULE glContainer

#include "testHelpers.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "glContainer.h"

BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::const_iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::reserve_iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::const_reserve_iterator);

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

BOOST_FIXTURE_TEST_SUITE(i, glContainer<int>)

BOOST_AUTO_TEST_CASE(createDestroy, *boost::unit_test::timeout(1))
{
	BOOST_CHECK(!data_);
	BOOST_CHECK_NO_THROW(map());
	BOOST_REQUIRE(data_);
	BOOST_CHECK_NO_THROW(unmap());
	BOOST_CHECK(!data_);
}

BOOST_AUTO_TEST_CASE(push_back_test, *boost::unit_test::timeout(1))
{
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 0);
	BOOST_CHECK_NO_THROW(push_back(1));
	BOOST_CHECK_NO_THROW(push_back(2));
	BOOST_CHECK_NO_THROW(push_back(3));
	BOOST_CHECK_NO_THROW(push_back(4));
	BOOST_CHECK_EQUAL(capacity_, 4);
	BOOST_CHECK_EQUAL(size_, 4);
	{
		std::array expected1 {1, 2, 3, 4};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(rbegin(), rend(), expected1.rbegin(), expected1.rend());
	}
}

BOOST_AUTO_TEST_CASE(emplace_back_test, *boost::unit_test::timeout(1))
{
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 0);

	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_EQUAL(capacity_, 2);
	BOOST_CHECK_EQUAL(size_, 2);

	BOOST_CHECK_NO_THROW(reserve(5));
	BOOST_CHECK_EQUAL(capacity_, 5);
	BOOST_CHECK_EQUAL(size_, 2);

	BOOST_CHECK_NO_THROW(emplace_back(3));
	BOOST_CHECK_NO_THROW(emplace_back(4));
	BOOST_CHECK_EQUAL(capacity_, 5);
	BOOST_CHECK_EQUAL(size_, 4);

	{
		std::array expected1 {1, 2, 3, 4};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(rbegin(), rend(), expected1.rbegin(), expected1.rend());
	}

	BOOST_CHECK_NO_THROW(emplace_back(5));
	BOOST_CHECK_EQUAL(capacity_, 5);
	BOOST_CHECK_EQUAL(size_, 5);
	BOOST_CHECK_NO_THROW(emplace_back(6));
	BOOST_CHECK_NO_THROW(emplace_back(7));
	BOOST_CHECK_EQUAL(capacity_, 7);
	BOOST_CHECK_EQUAL(size_, 7);

	{
		std::array expected2 {1, 2, 3, 4, 5, 6, 7};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected2.begin(), expected2.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(rbegin(), rend(), expected2.rbegin(), expected2.rend());
	}

	BOOST_CHECK_EQUAL(7, end() - begin());
	BOOST_CHECK_EQUAL(7, rend() - rbegin());
}

BOOST_AUTO_TEST_CASE(resize_test)
{
	BOOST_CHECK_NO_THROW(push_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_NO_THROW(resize(4));
	BOOST_CHECK_EQUAL(capacity_, 4);
	BOOST_CHECK_EQUAL(size_, 4);
	{
		std::array expected1 {1, 2, 0, 0};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}

	BOOST_CHECK_NO_THROW(resize(1));
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 1);
	{
		std::array expected2 {1};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected2.begin(), expected2.end());
	}

	BOOST_CHECK_NO_THROW(resize(1));
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 1);

	BOOST_CHECK_NO_THROW(resize(0));
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 0);
	BOOST_CHECK_EQUAL(begin(), end());
	BOOST_CHECK_EQUAL(rbegin(), rend());
}

BOOST_AUTO_TEST_CASE(shrink_to_fit_test)
{
	BOOST_CHECK_NO_THROW(reserve(4));
	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_EQUAL(capacity_, 4);
	BOOST_CHECK_EQUAL(size_, 2);
	{
		std::array expected1 {1, 2};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	BOOST_CHECK_NO_THROW(shrink_to_fit());
	BOOST_CHECK_EQUAL(capacity_, 2);
	BOOST_CHECK_EQUAL(size_, 2);
	{
		std::array expected1 {1, 2};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}

	BOOST_CHECK_NO_THROW(shrink_to_fit());
	BOOST_CHECK_EQUAL(capacity(), 2);
	BOOST_CHECK_EQUAL(size(), 2);

	BOOST_CHECK_NO_THROW(clear());
	BOOST_CHECK_EQUAL(capacity(), 2);
	BOOST_CHECK_EQUAL(size(), 0);
}

BOOST_AUTO_TEST_CASE(getters)
{
	BOOST_CHECK(empty());
	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK(!empty());
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_EQUAL(1, front());
	BOOST_CHECK_EQUAL(2, back());
	BOOST_CHECK_EQUAL(1, at(0));
	BOOST_CHECK_EQUAL(2, at(1));
	BOOST_CHECK_EQUAL(1, (*this)[0]);
	BOOST_CHECK_EQUAL(2, (*this)[1]);
	BOOST_CHECK_EQUAL(data_, data());
	BOOST_CHECK_THROW(std::ignore = at(2), std::out_of_range);

	const auto & constCont {*this};
	BOOST_CHECK_EQUAL(1, constCont.front());
	BOOST_CHECK_EQUAL(2, constCont.back());
	{
		std::array expected1 {1, 2};
		BOOST_CHECK_EQUAL_COLLECTIONS(constCont.begin(), constCont.end(), expected1.begin(), expected1.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(constCont.rbegin(), constCont.rend(), expected1.rbegin(), expected1.rend());
		BOOST_CHECK_EQUAL_COLLECTIONS(constCont.cbegin(), constCont.cend(), expected1.cbegin(), expected1.cend());
		BOOST_CHECK_EQUAL_COLLECTIONS(constCont.crbegin(), constCont.crend(), expected1.crbegin(), expected1.crend());
	}
	BOOST_CHECK_EQUAL(1, constCont.at(0));
	BOOST_CHECK_EQUAL(2, constCont.at(1));
	BOOST_CHECK_EQUAL(1, constCont[0]);
	BOOST_CHECK_EQUAL(2, constCont[1]);
	BOOST_CHECK_EQUAL(data_, constCont.data());
	BOOST_CHECK_THROW(std::ignore = constCont.at(2), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(random_access)
{
	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_NO_THROW(emplace_back(3));

	auto i = begin();
	BOOST_CHECK_EQUAL(1, *i);
	BOOST_CHECK_EQUAL(2, *++i);
	BOOST_CHECK_EQUAL(2, *i++);
	BOOST_CHECK_EQUAL(3, *i);
	BOOST_CHECK_EQUAL(3, *i--);
	BOOST_CHECK_EQUAL(2, *i);
	BOOST_CHECK_EQUAL(1, *--i);
	BOOST_CHECK_EQUAL(1, *i);
}

BOOST_AUTO_TEST_SUITE_END();

struct C {
	int x;
	float y;
};

BOOST_FIXTURE_TEST_SUITE(c, glContainer<C>)

BOOST_AUTO_TEST_CASE(basic)
{
	BOOST_CHECK_NO_THROW(emplace_back(1, 2.f));
	BOOST_CHECK_EQUAL(1, begin()->x);
	BOOST_CHECK_EQUAL(2.f, begin()->y);
	BOOST_CHECK_NO_THROW(begin()->x = 3);
	BOOST_CHECK_EQUAL(3, begin()->x);

	BOOST_CHECK_NO_THROW(push_back(C {4, 5.f}));
	BOOST_CHECK_EQUAL(3, begin()->x);
	BOOST_CHECK_EQUAL(2.f, begin()->y);
	BOOST_CHECK_EQUAL(4, rbegin()->x);
	BOOST_CHECK_EQUAL(5.f, rbegin()->y);
}

BOOST_AUTO_TEST_SUITE_END();
