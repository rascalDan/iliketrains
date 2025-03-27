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

	[[nodiscard]] virtual bool
	yes() const
	{
		return true;
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

class Sub1 : public Sub { };

class Sub2 : public Sub { };

class Base2 {
public:
	virtual ~Base2() = default;
};

class Multi : public Sub1, public Base2 { };

using TestCollection = SharedCollection<Base, Sub>;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_reverse_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::OtherObjects<Sub>::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::OtherObjects<Sub>::const_iterator)

BOOST_FIXTURE_TEST_SUITE(tc, TestCollection)

BOOST_AUTO_TEST_CASE(empty)
{
	BOOST_CHECK(TestCollection::empty());
	BOOST_REQUIRE(!apply(&Base::add));
	const auto i = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(i, end());
	BOOST_CHECK(!find<Base>());
	BOOST_CHECK(!find<Sub>());
	BOOST_CHECK(!find<Sub1>());
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
	BOOST_CHECK_EQUAL(b.get(), find<Base>());
	BOOST_CHECK(!find<Sub>());
	BOOST_CHECK(!find<Sub1>());
}

BOOST_AUTO_TEST_CASE(emplace_others)
{
	auto b = emplace(std::make_shared<Base>());
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	auto s = emplace(std::make_shared<Sub>());
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
	BOOST_CHECK_EQUAL(b.get(), find<Base>());
	BOOST_CHECK_EQUAL(s.get(), find<Sub>());
	BOOST_CHECK(!find<Sub1>());
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

BOOST_AUTO_TEST_CASE(filter)
{
	static_assert(TestCollection::idx<Sub>() == 0);
	static_assert(TestCollection::idx<const Sub>() == 0);
	static_assert(TestCollection::idx<Sub1>() == 0);
	static_assert(TestCollection::idx<const Sub1>() == 0);
	create<Base>();
	BOOST_CHECK_EQUAL(1, apply<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(0, apply<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(0, apply<Sub1>(&Base::yes));
	BOOST_CHECK_EQUAL(objects.begin(), applyOne<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).end(), applyOne<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).end(), applyOne<Sub1>(&Base::yes));
	create<Sub>();
	BOOST_CHECK_EQUAL(2, apply<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(1, apply<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(0, apply<Sub1>(&Base::yes));
	BOOST_CHECK_EQUAL(objects.begin(), applyOne<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).begin(), applyOne<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).end(), applyOne<Sub1>(&Base::yes));
	create<Sub1>();
	BOOST_CHECK_EQUAL(3, apply<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(2, apply<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(1, apply<Sub1>(&Base::yes));
	BOOST_CHECK_EQUAL(objects.begin(), applyOne<Base>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).begin(), applyOne<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).begin() + 1, applyOne<Sub1>(&Base::yes));

	BOOST_CHECK_EQUAL(std::get<idx<Sub>()>(otherObjects).size(), 2);
	BOOST_CHECK_EQUAL(std::get<idx<Sub1>()>(otherObjects).size(), 2);

	BOOST_CHECK_EQUAL(&objects, &containerFor<Base>());
	BOOST_CHECK_EQUAL(&std::get<0>(otherObjects), &containerFor<Sub>());
	BOOST_CHECK_EQUAL(&std::get<0>(otherObjects), &containerFor<Sub1>());
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

BOOST_AUTO_TEST_CASE(createCreate)
{
	auto b = findOrCreate<Base>();
	BOOST_CHECK(b);
	auto b2 = findOrCreate<Base>();
	BOOST_CHECK_EQUAL(b, b2);
	auto s = findOrCreate<Sub>();
	BOOST_CHECK_NE(s, b);
	auto s2 = findOrCreate<Sub>();
	BOOST_CHECK_EQUAL(s, s2);
}

BOOST_AUTO_TEST_CASE(createCreateSub)
{
	auto s = findOrCreate<Sub>();
	auto b = findOrCreate<Base>();
	BOOST_CHECK_EQUAL(s, b);
}

BOOST_AUTO_TEST_SUITE_END()

using TestUniqueCollection = UniqueCollection<Base, Sub>;
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_reverse_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::reverse_iterator)

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

BOOST_AUTO_TEST_CASE(clearAll)
{
	create<Base>();
	create<Sub>();
	emplace(std::make_unique<Base>());
	emplace(std::make_unique<Sub>());

	clear();
	BOOST_CHECK(objects.empty());
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
}

BOOST_AUTO_TEST_CASE(removeAllOfSub)
{
	create<Base>();
	create<Sub>();
	emplace(std::make_unique<Base>());
	emplace(std::make_unique<Sub>());
	emplace(std::make_unique<Sub1>());

	BOOST_CHECK_EQUAL(removeAll<Sub>(), 3);
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
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

BOOST_AUTO_TEST_CASE(applyAll)
{
	create<Base>();
	BOOST_CHECK_EQUAL(0, apply<Sub>(&Base::add));
	BOOST_CHECK_EQUAL(1, apply<Base>(&Base::add));
	create<Sub>();
	BOOST_CHECK_EQUAL(1, apply<Sub>(&Base::add));
	BOOST_CHECK_EQUAL(2, apply<Base>(&Base::add));
}

BOOST_AUTO_TEST_CASE(applyOneType)
{
	create<Base>();
	BOOST_CHECK_EQUAL(objects.end(), applyOne<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(objects.begin(), applyOne<Base>(&Base::yes));
	create<Sub>();
	BOOST_CHECK_EQUAL(objects.begin() + 1, applyOne<Sub>(&Base::yes));
	BOOST_CHECK_EQUAL(objects.begin(), applyOne<Base>(&Base::yes));
	create<Sub1>();
	BOOST_CHECK_EQUAL(objects.begin() + 2, applyOne<Sub1>(&Base::yes));
}

BOOST_AUTO_TEST_SUITE_END()

using MultiCollection = Collection<std::unique_ptr<Base>, Multi, Sub, Base2>;

BOOST_FIXTURE_TEST_SUITE(multi, MultiCollection)

BOOST_AUTO_TEST_CASE(addMulti)
{
	static_assert(MultiCollection::idx<Multi>() == 0);
	static_assert(MultiCollection::idx<Sub>() == 1);
	static_assert(MultiCollection::idx<Base2>() == 2);
	create<Base>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);
	create<Sub>();
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 1);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);
	create<Sub1>();
	BOOST_CHECK_EQUAL(objects.size(), 3);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 2);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);
	create<Sub2>();
	BOOST_CHECK_EQUAL(objects.size(), 4);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 3);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);
	create<Multi>();
	BOOST_CHECK_EQUAL(objects.size(), 5);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 1);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 4);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 1);
}

BOOST_AUTO_TEST_CASE(removeMulti)
{
	create<Base>();
	create<Sub>();
	create<Sub1>();
	create<Sub2>();
	create<Multi>();
	BOOST_CHECK_EQUAL(objects.size(), 5);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 1);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 4);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 1);

	BOOST_CHECK_EQUAL(removeAll<Multi>(), 1);
	BOOST_CHECK_EQUAL(objects.size(), 4);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 3);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);

	BOOST_CHECK_EQUAL(removeAll<Sub>(), 3);
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);

	BOOST_CHECK_EQUAL(removeAll<Base>(), 1);
	BOOST_CHECK_EQUAL(objects.size(), 0);
	BOOST_CHECK_EQUAL(std::get<0>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<1>(otherObjects).size(), 0);
	BOOST_CHECK_EQUAL(std::get<2>(otherObjects).size(), 0);
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
