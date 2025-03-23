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

using TestCollection = SharedCollection<Base, Sub>;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_reverse_iterator)

BOOST_FIXTURE_TEST_SUITE(tc, TestCollection)

BOOST_AUTO_TEST_CASE(empty)
{
	BOOST_CHECK(TestCollection::empty());
	BOOST_REQUIRE(!apply(&Base::add));
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, end());
}

BOOST_AUTO_TEST_CASE(a_base)
{
	auto b = create<Base>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(b->total, 1);
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, end());
}

BOOST_AUTO_TEST_CASE(emplace_others)
{
	emplace(std::make_shared<Base>());
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	emplace(std::make_shared<Sub>());
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
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
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(s->total, 2);
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_NE(i, end());
	BOOST_CHECK_EQUAL(*i, s);
}

BOOST_AUTO_TEST_CASE(begin_end)
{
	BOOST_CHECK_EQUAL(0, std::distance(begin(), end()));
	create<Sub>();
	create<Base>();
	BOOST_CHECK_EQUAL(2, std::distance(begin(), end()));
}

BOOST_AUTO_TEST_CASE(rbegin_rend)
{
	BOOST_CHECK_EQUAL(0, std::distance(rbegin(), rend()));
	create<Sub>();
	create<Base>();
	BOOST_CHECK_EQUAL(2, std::distance(rbegin(), rend()));
}

BOOST_AUTO_TEST_SUITE_END()

using TestUniqueCollection = UniqueCollection<Base, Sub>;
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_reverse_iterator)

BOOST_FIXTURE_TEST_SUITE(utc, TestUniqueCollection)

BOOST_AUTO_TEST_CASE(unique_create)
{
	create<Base>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	create<Sub>();
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
}

BOOST_AUTO_TEST_CASE(move_assign)
{
	create<Base>();
	create<Sub>();

	TestUniqueCollection::Objects other;
	TestUniqueCollection::operator=(std::move(other));
	BOOST_CHECK(objects.empty());
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());

	other.push_back(std::make_unique<Sub>());
	other.push_back(std::make_unique<Base>());
	TestUniqueCollection::operator=(std::move(other));
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
	BOOST_CHECK(other.empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(btc, UniqueCollection<Base>)

BOOST_AUTO_TEST_CASE(no_others)
{
	create<Base>();
	create<Sub>();
	emplace(std::make_unique<Base>());
	emplace(std::make_unique<Sub>());
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
