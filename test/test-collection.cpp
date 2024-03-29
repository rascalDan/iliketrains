#define BOOST_TEST_MODULE test_collection

#include <boost/test/unit_test.hpp>

#include <collection.h>
#include <memory>
#include <ptr.h>
#include <special_members.h>
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

BOOST_TEST_DONT_PRINT_LOG_VALUE(Collection<Base>::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Collection<Base>::Objects::const_reverse_iterator)

BOOST_FIXTURE_TEST_SUITE(tc, TestCollection)

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

BOOST_AUTO_TEST_CASE(a_rbase)
{
	auto b = create<Base>();
	BOOST_REQUIRE(rapply(&Base::add));
	BOOST_CHECK_EQUAL(b->total, 1);
	const auto i = rapplyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, rend());
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(wrapped_ptr_file_cons)
{
	using FilePtr = wrapped_ptr<FILE, &fclose>;
	const FilePtr fp {fopen, "/dev/null", "r"};
	BOOST_REQUIRE(fp);
	BOOST_CHECK_NO_THROW(fflush(fp));

	BOOST_CHECK_EQUAL(fp.get(), fp.operator->());
	BOOST_CHECK_EQUAL(fp.get(), fp.operator FILE *());
}

BOOST_AUTO_TEST_CASE(wrapped_ptr_file_move)
{
	using FilePtr = wrapped_ptr<FILE, &fclose>;
	FilePtr fp {fopen, "/dev/null", "r"};
	BOOST_REQUIRE(fp);

	FilePtr fp2 {std::move(fp)};
	BOOST_REQUIRE(!fp);
	BOOST_REQUIRE(fp2);

	fp = std::move(fp2);
	BOOST_REQUIRE(fp);
	BOOST_REQUIRE(!fp2);

	FilePtr fp3 {fopen, "/dev/null", "r"};
	fp = std::move(fp3);
}

BOOST_AUTO_TEST_CASE(wrapped_ptr_file_typed)
{
	using FilePtr = wrapped_ptrt<FILE, &fopen, &fclose>;
	const FilePtr fp {"/dev/null", "r"};
	BOOST_REQUIRE(fp);
	BOOST_CHECK_NO_THROW(fflush(fp));
}
