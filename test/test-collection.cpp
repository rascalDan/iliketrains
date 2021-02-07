#define BOOST_TEST_MODULE test_collection

#include <boost/test/unit_test.hpp>

#include <collection.hpp>
#include <memory>
#include <special_members.hpp>
#include <vector>

class Base {
public:
	Base() = default;
	virtual ~Base() = default;
	DEFAULT_MOVE_COPY(Base);
	virtual bool
	add()
	{
		total += 1;
		return false;
	}
	unsigned int total {0};
};

class Sub : public Base {
public:
	bool
	add() override
	{
		total += 2;
		return true;
	}
};

using TestCollection = Collection<Base>;

BOOST_TEST_DONT_PRINT_LOG_VALUE(Collection<Base>::Objects::const_iterator);

BOOST_FIXTURE_TEST_SUITE(tc, TestCollection);

BOOST_AUTO_TEST_CASE(empty)
{
	BOOST_REQUIRE(!apply(&Base::add));
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, end());
}

BOOST_AUTO_TEST_CASE(a_base)
{
	auto b = create<Base>();
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(b->total, 1);
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, end());
}

BOOST_AUTO_TEST_CASE(a_sub)
{
	auto s = create<Sub>();
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(s->total, 2);
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_NE(i, end());
	BOOST_CHECK_EQUAL(*i, s);
}

BOOST_AUTO_TEST_SUITE_END();
