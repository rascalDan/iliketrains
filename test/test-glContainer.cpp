#define BOOST_TEST_MODULE glContainer

#include "testMainWindow.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "glContainer.h"

// Force generation of all functions
template class glContainer<int>;

BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::const_iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::reverse_iterator);
BOOST_TEST_DONT_PRINT_LOG_VALUE(glContainer<int>::const_reverse_iterator);

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

BOOST_FIXTURE_TEST_SUITE(i, glContainer<int>)

BOOST_AUTO_TEST_CASE(CreateDestroy, *boost::unit_test::timeout(1))
{
	// Unmapped
	BOOST_CHECK(!data_);
	BOOST_CHECK(!access_);
	// Request map, but empty
	BOOST_CHECK_NO_THROW(map(GL_READ_ONLY));
	BOOST_REQUIRE(!data_);
	BOOST_REQUIRE(!access_);
	BOOST_CHECK_NO_THROW(map(GL_READ_WRITE));
	BOOST_REQUIRE(!data_);
	BOOST_REQUIRE(!access_);
	// Add something
	BOOST_CHECK_NO_THROW(emplace_back(0));
	BOOST_REQUIRE(data_);
	BOOST_REQUIRE_EQUAL(access_, GL_READ_WRITE);
	// Unmap
	BOOST_CHECK_NO_THROW(unmap());
	BOOST_REQUIRE(!data_);
	BOOST_REQUIRE(!access_);
	// Map RO
	BOOST_CHECK_NO_THROW(map(GL_READ_ONLY));
	BOOST_REQUIRE(data_);
	BOOST_REQUIRE_EQUAL(access_, GL_READ_ONLY);
	// Map RW upgradde
	BOOST_CHECK_NO_THROW(map(GL_READ_WRITE));
	BOOST_REQUIRE(data_);
	BOOST_REQUIRE_EQUAL(access_, GL_READ_WRITE);
	// Map RO downgradde, no change
	BOOST_CHECK_NO_THROW(map(GL_READ_ONLY));
	BOOST_REQUIRE(data_);
	BOOST_REQUIRE_EQUAL(access_, GL_READ_WRITE);
	// Unmap
	BOOST_CHECK_NO_THROW(unmap());
	BOOST_CHECK(!data_);
	BOOST_CHECK(!access_);
}

BOOST_AUTO_TEST_CASE(MapModes)
{
	BOOST_CHECK_EQUAL(std::accumulate(begin(), end(), 0), 0);
	BOOST_CHECK(!data_);
	BOOST_CHECK(!access_);

	BOOST_CHECK_NO_THROW(push_back(1));
	BOOST_CHECK_NO_THROW(push_back(2));
	BOOST_CHECK_NO_THROW(push_back(3));
	BOOST_CHECK_NO_THROW(push_back(4));
	BOOST_CHECK_NO_THROW(unmap());

	BOOST_CHECK_EQUAL(std::accumulate(cbegin(), cend(), 0), 10);
	BOOST_CHECK(data_);
	BOOST_CHECK_EQUAL(access_, GL_READ_ONLY);
	BOOST_CHECK_NO_THROW(unmap());

	BOOST_CHECK_EQUAL(std::accumulate(begin(), end(), 0), 10);
	BOOST_CHECK(data_);
	BOOST_CHECK_EQUAL(access_, GL_READ_WRITE);
	BOOST_CHECK_NO_THROW(unmap());

	BOOST_CHECK_EQUAL(std::ranges::fold_left(std::as_const(*this), 0, std::plus {}), 10);
	BOOST_CHECK(data_);
	BOOST_CHECK_EQUAL(access_, GL_READ_ONLY);
	BOOST_CHECK_NO_THROW(unmap());
}

BOOST_AUTO_TEST_CASE(PushBackTest, *boost::unit_test::timeout(1))
{
	BOOST_CHECK_EQUAL(capacity_, 1);
	BOOST_CHECK_EQUAL(size_, 0);
	BOOST_CHECK_NO_THROW(push_back(1));
	BOOST_CHECK_NO_THROW(push_back(2));
	BOOST_CHECK_NO_THROW(push_back(3));
	BOOST_CHECK_NO_THROW(push_back(4));
	BOOST_CHECK_NO_THROW(push_back(5));
	BOOST_CHECK_EQUAL(capacity_, 8);
	BOOST_CHECK_EQUAL(size_, 5);
	{
		std::array expected1 {1, 2, 3, 4, 5};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(rbegin(), rend(), expected1.rbegin(), expected1.rend());
	}
}

BOOST_AUTO_TEST_CASE(EmplaceBackTest, *boost::unit_test::timeout(1))
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
	BOOST_CHECK_EQUAL(capacity_, 10);
	BOOST_CHECK_EQUAL(size_, 7);

	{
		std::array expected2 {1, 2, 3, 4, 5, 6, 7};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected2.begin(), expected2.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(rbegin(), rend(), expected2.rbegin(), expected2.rend());
	}

	BOOST_CHECK_EQUAL(7, end() - begin());
	BOOST_CHECK_EQUAL(7, rend() - rbegin());
}

BOOST_AUTO_TEST_CASE(ResizeTest)
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

BOOST_AUTO_TEST_CASE(ShrinkToFitTest)
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

BOOST_AUTO_TEST_CASE(Getters)
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

BOOST_AUTO_TEST_CASE(RandomAccess)
{
	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_NO_THROW(emplace_back(3));

	auto iterator = begin();
	BOOST_CHECK_EQUAL(1, *iterator);
	BOOST_CHECK_EQUAL(2, *++iterator);
	BOOST_CHECK_EQUAL(2, *iterator++);
	BOOST_CHECK_EQUAL(3, *iterator);
	BOOST_CHECK_EQUAL(3, *iterator--);
	BOOST_CHECK_EQUAL(2, *iterator);
	BOOST_CHECK_EQUAL(1, *--iterator);
	BOOST_CHECK_EQUAL(1, *iterator);
}

BOOST_AUTO_TEST_CASE(RandomWrite)
{
	BOOST_CHECK_NO_THROW(resize(3));
	BOOST_CHECK_EQUAL(size(), 3);
	BOOST_CHECK_NO_THROW(unmap());
	BOOST_REQUIRE(!data_);
	BOOST_CHECK_NO_THROW(at(0, 10));
	BOOST_CHECK_NO_THROW(at(1, 20));
	BOOST_CHECK_NO_THROW(at(2, 30));
	BOOST_CHECK(!data_);
	{
		std::array expected1 {10, 20, 30};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	BOOST_CHECK(data_);
	BOOST_CHECK_NO_THROW(at(1, 40));
	{
		std::array expected1 {10, 40, 30};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	BOOST_CHECK_THROW(at(4, 0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(InsertRemoveTest)
{
	BOOST_CHECK_NO_THROW(emplace_back(1));
	BOOST_CHECK_NO_THROW(emplace_back(2));
	BOOST_CHECK_NO_THROW(emplace_back(3));
	BOOST_CHECK_NO_THROW(emplace_back(4));
	BOOST_CHECK_NO_THROW(pop_back());
	BOOST_CHECK_EQUAL(size_, 3);
	BOOST_CHECK_EQUAL(capacity_, 4);

	BOOST_CHECK_NO_THROW(emplace(begin(), 5));
	BOOST_CHECK_EQUAL(size_, 4);
	BOOST_CHECK_EQUAL(capacity_, 4);
	{
		std::array expected1 {5, 1, 2, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}

	{
		std::array expected1 {2, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin() + 2, end(), expected1.begin(), expected1.end());
	}
	BOOST_CHECK_NO_THROW(insert(begin() + 2, 6));
	BOOST_CHECK_EQUAL(size_, 5);
	BOOST_CHECK_EQUAL(capacity_, 8);
	{
		std::array expected1 {5, 1, 6, 2, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	erase(begin() + 1);
	BOOST_CHECK_EQUAL(size_, 4);
	BOOST_CHECK_EQUAL(capacity_, 8);
	{
		std::array expected1 {5, 6, 2, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	erase(begin() + 1, end() - 1);
	BOOST_CHECK_EQUAL(size_, 2);
	BOOST_CHECK_EQUAL(capacity_, 8);
	{
		std::array expected1 {5, 3};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
}

BOOST_AUTO_TEST_CASE(StlCompatilibty)
{
	BOOST_CHECK_NO_THROW(resize(10));
	BOOST_CHECK_NO_THROW(std::generate(begin(), end(), [value = 0]() mutable {
		return value++;
	}));
	BOOST_CHECK_NO_THROW(std::sort(rbegin(), rend()));
	{
		std::array expected1 {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), end(), expected1.begin(), expected1.end());
	}
	const auto newend = std::remove_if(begin(), end(), [](const auto & value) {
		return value % 2 == 0;
	});
	{
		std::array expected1 {9, 7, 5, 3, 1};
		BOOST_CHECK_EQUAL_COLLECTIONS(begin(), newend, expected1.begin(), expected1.end());
	}
}

BOOST_AUTO_TEST_CASE(IterCompare)
{
	BOOST_CHECK_EQUAL(begin(), end());
	BOOST_CHECK_EQUAL(rbegin(), rend());
	emplace_back();
	BOOST_CHECK_LT(begin(), end());
	BOOST_CHECK_LT(rbegin(), rend());
	BOOST_CHECK_LT(cbegin(), cend());
	BOOST_CHECK_LT(crbegin(), crend());
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE(CreateCopySource, *boost::unit_test::timeout(1))
{
	const std::vector src {4, 6, 2, 4, 6, 0};
	glContainer dst {src};
	static_assert(std::is_same_v<decltype(src)::value_type, decltype(dst)::value_type>);
	dst.unmap();
	BOOST_CHECK_EQUAL_COLLECTIONS(src.begin(), src.end(), dst.begin(), dst.end());
}

namespace {
	struct C {
		int x;
		float y;
	};

	static_assert(std::is_trivially_destructible_v<C>);

	struct CC {
		CC() = default;

		CC(int intValue, float floatValue) noexcept : x {intValue}, y {floatValue} { }

		~CC()
		{
			++x;
		}

		DEFAULT_MOVE_COPY(CC);

		int x;
		float y;
	};

	static_assert(!std::is_trivially_destructible_v<CC>);
}

BOOST_FIXTURE_TEST_SUITE(c, glContainer<C>)

BOOST_AUTO_TEST_CASE(Basic)
{
	BOOST_CHECK_NO_THROW(emplace_back(1, 2.F));
	BOOST_CHECK_EQUAL(1, begin()->x);
	BOOST_CHECK_EQUAL(2.F, begin()->y);
	BOOST_CHECK_NO_THROW(begin()->x = 3);
	BOOST_CHECK_EQUAL(3, begin()->x);

	BOOST_CHECK_NO_THROW(push_back(C {4, 5.F}));
	BOOST_CHECK_EQUAL(3, begin()->x);
	BOOST_CHECK_EQUAL(2.F, begin()->y);
	BOOST_CHECK_EQUAL(4, rbegin()->x);
	BOOST_CHECK_EQUAL(5.F, rbegin()->y);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE(cc, glContainer<CC>)

BOOST_AUTO_TEST_CASE(Basic)
{
	BOOST_CHECK_NO_THROW(emplace_back(1, 2.F));
	BOOST_CHECK_EQUAL(1, begin()->x);
	BOOST_CHECK_EQUAL(2.F, begin()->y);
	BOOST_CHECK_NO_THROW(begin()->x = 3);
	BOOST_CHECK_EQUAL(3, begin()->x);

	BOOST_CHECK_NO_THROW(push_back(CC {4, 5.F}));
	BOOST_CHECK_EQUAL(3, begin()->x);
	BOOST_CHECK_EQUAL(2.F, begin()->y);
	BOOST_CHECK_EQUAL(4, rbegin()->x);
	BOOST_CHECK_EQUAL(5.F, rbegin()->y);
	BOOST_CHECK_NO_THROW(pop_back());
	BOOST_CHECK_EQUAL(size(), 1);
	BOOST_CHECK_EQUAL(capacity(), 2);
	BOOST_CHECK_NO_THROW(resize(3));
	BOOST_CHECK_EQUAL(size(), 3);
	BOOST_CHECK_EQUAL(capacity(), 3);
	BOOST_CHECK_NO_THROW(resize(1));
	BOOST_CHECK_EQUAL(size(), 1);
	BOOST_CHECK_EQUAL(capacity(), 1);
}

BOOST_AUTO_TEST_CASE(InsertRemoveTest)
{
	BOOST_CHECK_NO_THROW(emplace_back(1, 2.F));
	BOOST_CHECK_NO_THROW(emplace_back(3, 4.F));
	BOOST_CHECK_NO_THROW(emplace(begin(), 5, 6.F));
	BOOST_CHECK_NO_THROW(emplace(begin() + 1, 7, 8.F));
	BOOST_CHECK_NO_THROW(emplace(begin() + 2, 9, 10.F));
	BOOST_CHECK_EQUAL(capacity(), 8);
	BOOST_CHECK_EQUAL(size(), 5);
	BOOST_CHECK_NO_THROW(shrink_to_fit());
	BOOST_CHECK_EQUAL(capacity(), 5);
	BOOST_CHECK_EQUAL(size(), 5);
}

BOOST_AUTO_TEST_SUITE_END();
