#define BOOST_TEST_MODULE test_collection

#include <boost/test/unit_test.hpp>

#include <collection.h>
#include <memory>
#include <ptr.h>
#include <special_members.h>
#include <vector>

namespace {
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
		Base2() = default;
		virtual ~Base2() = default;
		DEFAULT_MOVE_COPY(Base2);
	};

	class Multi : public Sub1, public Base2 { };

	using TestCollection = SharedCollection<Base, Sub>;
}

BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::Objects::const_reverse_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::OtherObjects<Sub>::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestCollection::OtherObjects<Sub>::const_iterator)

BOOST_FIXTURE_TEST_SUITE(tc, TestCollection)

BOOST_AUTO_TEST_CASE(Empty)
{
	BOOST_CHECK(TestCollection::empty());
	BOOST_REQUIRE(!apply(&Base::add));
	const auto appliedTo = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(appliedTo, end());
	BOOST_CHECK(!find<Base>());
	BOOST_CHECK(!find<Sub>());
	BOOST_CHECK(!find<Sub1>());
}

BOOST_AUTO_TEST_CASE(ABaseApply)
{
	auto base = create<Base>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(base->total, 1);
	const auto appliedTo = applyOne(&Base::add);
	BOOST_CHECK_EQUAL(appliedTo, end());
	BOOST_CHECK_EQUAL(base.get(), find<Base>());
	BOOST_CHECK(!find<Sub>());
	BOOST_CHECK(!find<Sub1>());
}

BOOST_AUTO_TEST_CASE(EmplaceOthers)
{
	auto base = emplace(std::make_shared<Base>());
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	auto sub = emplace(std::make_shared<Sub>());
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
	BOOST_CHECK_EQUAL(base.get(), find<Base>());
	BOOST_CHECK_EQUAL(sub.get(), find<Sub>());
	BOOST_CHECK(!find<Sub1>());
}

BOOST_AUTO_TEST_CASE(ABaseRApply)
{
	auto base = create<Base>();
	BOOST_REQUIRE(rapply(&Base::add));
	BOOST_CHECK_EQUAL(base->total, 1);
	const auto appliedTo = rapplyOne(&Base::add);
	BOOST_CHECK_EQUAL(appliedTo, rend());
}

BOOST_AUTO_TEST_CASE(ASubApply)
{
	auto sub = create<Sub>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
	BOOST_REQUIRE(apply(&Base::add));
	BOOST_CHECK_EQUAL(sub->total, 2);
	const auto appliedTo = applyOne(&Base::add);
	BOOST_CHECK_NE(appliedTo, end());
	BOOST_CHECK_EQUAL(*appliedTo, sub);
}

BOOST_AUTO_TEST_CASE(Filter)
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

BOOST_AUTO_TEST_CASE(BeginEnd)
{
	BOOST_CHECK_EQUAL(0, std::distance(begin(), end()));
	create<Sub>();
	create<Base>();
	BOOST_CHECK_EQUAL(2, std::distance(begin(), end()));
}

BOOST_AUTO_TEST_CASE(RBeginREnd)
{
	BOOST_CHECK_EQUAL(0, std::distance(rbegin(), rend()));
	create<Sub>();
	create<Base>();
	BOOST_CHECK_EQUAL(2, std::distance(rbegin(), rend()));
}

BOOST_AUTO_TEST_CASE(CreateCreate)
{
	auto base1 = findOrCreate<Base>();
	BOOST_CHECK(base1);
	auto base2 = findOrCreate<Base>();
	BOOST_CHECK_EQUAL(base1, base2);
	auto sub1 = findOrCreate<Sub>();
	BOOST_CHECK_NE(sub1, base1);
	auto sub2 = findOrCreate<Sub>();
	BOOST_CHECK_EQUAL(sub1, sub2);
}

BOOST_AUTO_TEST_CASE(CreateCreateSub)
{
	auto sub = findOrCreate<Sub>();
	auto base = findOrCreate<Base>();
	BOOST_CHECK_EQUAL(sub, base);
}

BOOST_AUTO_TEST_SUITE_END()

using TestUniqueCollection = UniqueCollection<Base, Sub>;
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::const_reverse_iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestUniqueCollection::Objects::reverse_iterator)

BOOST_FIXTURE_TEST_SUITE(utc, TestUniqueCollection)

BOOST_AUTO_TEST_CASE(UniqueCreate)
{
	create<Base>();
	BOOST_CHECK_EQUAL(objects.size(), 1);
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
	create<Sub>();
	BOOST_CHECK_EQUAL(objects.size(), 2);
	BOOST_CHECK_EQUAL(std::get<OtherObjects<Sub>>(otherObjects).size(), 1);
}

BOOST_AUTO_TEST_CASE(MoveAssign)
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

BOOST_AUTO_TEST_CASE(ClearAll)
{
	create<Base>();
	create<Sub>();
	emplace(std::make_unique<Base>());
	emplace(std::make_unique<Sub>());

	clear();
	BOOST_CHECK(objects.empty());
	BOOST_CHECK(std::get<OtherObjects<Sub>>(otherObjects).empty());
}

BOOST_AUTO_TEST_CASE(RemoveAllOfSub)
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

BOOST_AUTO_TEST_CASE(NoOthers)
{
	create<Base>();
	create<Sub>();
	emplace(std::make_unique<Base>());
	emplace(std::make_unique<Sub>());
}

BOOST_AUTO_TEST_CASE(ApplyAll)
{
	create<Base>();
	BOOST_CHECK_EQUAL(0, apply<Sub>(&Base::add));
	BOOST_CHECK_EQUAL(1, apply<Base>(&Base::add));
	create<Sub>();
	BOOST_CHECK_EQUAL(1, apply<Sub>(&Base::add));
	BOOST_CHECK_EQUAL(2, apply<Base>(&Base::add));
}

BOOST_AUTO_TEST_CASE(ApplyOneType)
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

BOOST_AUTO_TEST_CASE(AddMulti)
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
	BOOST_CHECK_EQUAL(size(), 5);
	BOOST_CHECK_EQUAL(size<Multi>(), 1);
	BOOST_CHECK_EQUAL(size<Sub>(), 4);
	BOOST_CHECK_EQUAL(size<Base2>(), 1);
}

BOOST_AUTO_TEST_CASE(RemoveMulti)
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

BOOST_AUTO_TEST_CASE(WrappedPtrFileCons)
{
	using FilePtr = wrapped_ptr<FILE, &fclose>;
	const FilePtr file {fopen, "/dev/null", "r"};
	BOOST_REQUIRE(file);
	BOOST_CHECK_NO_THROW(fflush(file));

	BOOST_CHECK_EQUAL(file.get(), file.operator->());
	BOOST_CHECK_EQUAL(file.get(), file.operator FILE *());
}

BOOST_AUTO_TEST_CASE(WrappedPtrFileMove)
{
	using FilePtr = wrapped_ptr<FILE, &fclose>;
	FilePtr file {fopen, "/dev/null", "r"};
	BOOST_REQUIRE(file);

	FilePtr fp2 {std::move(file)};
	BOOST_REQUIRE(!file);
	BOOST_REQUIRE(fp2);

	file = std::move(fp2);
	BOOST_REQUIRE(file);
	BOOST_REQUIRE(!fp2);

	FilePtr fp3 {fopen, "/dev/null", "r"};
	file = std::move(fp3);
}

BOOST_AUTO_TEST_CASE(WrappedPtrFileTyped)
{
	using FilePtr = wrapped_ptrt<FILE, &fopen, &fclose>;
	const FilePtr file {"/dev/null", "r"};
	BOOST_REQUIRE(file);
	BOOST_CHECK_NO_THROW(fflush(file));
}
