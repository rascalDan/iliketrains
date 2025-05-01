#define BOOST_TEST_MODULE test_persistence

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "testStructures.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <iosfwd>
#include <jsonParse-persistence.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace {
	struct JPP {
		template<typename T>
		T
		loadJson(const std::filesystem::path & path)
		{
			BOOST_TEST_CONTEXT(path) {
				std::ifstream inputStream {path};
				auto object = Persistence::JsonParsePersistence {}.loadState<T>(inputStream);
				BOOST_REQUIRE(object);
				return object;
			}

			// Presumably BOOST_TEST_CONTEXT is implemented as an if (...) { }
			std::unreachable();
		}
	};

	std::vector<std::filesystem::path>
	fixturesIn(const std::filesystem::path & root)
	{
		return {std::filesystem::directory_iterator {root}, {}};
	}
}

BOOST_FIXTURE_TEST_CASE(LoadObject, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/load_object.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_CHECK_CLOSE(object->flt, 3.14, 0.01);
	BOOST_CHECK_EQUAL(object->str, "Lovely string");
	BOOST_CHECK_EQUAL(object->bl, true);
	BOOST_CHECK_CLOSE(object->pos[0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(object->pos[1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(object->pos[2], 1.57, 0.01);
	BOOST_CHECK_EQUAL(object->gpos[0], 2147483647);
	BOOST_CHECK_EQUAL(object->gpos[1], 2147483646);
	BOOST_CHECK_EQUAL(object->gpos[2], -2147483648);
	BOOST_REQUIRE_EQUAL(object->flts.size(), 6);
	BOOST_CHECK_CLOSE(object->flts[0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(object->flts[1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(object->flts[2], 1.57, 0.01);
	BOOST_CHECK_CLOSE(object->flts[3], 0, 0.01);
	BOOST_CHECK_CLOSE(object->flts[4], -1, 0.01);
	BOOST_CHECK_CLOSE(object->flts[5], -3.14, 0.01);
	BOOST_REQUIRE_EQUAL(object->poss.size(), 2);
	BOOST_CHECK_CLOSE(object->poss[0][0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(object->poss[0][1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(object->poss[0][2], 1.57, 0.01);
	BOOST_CHECK_CLOSE(object->poss[1][0], 0, 0.01);
	BOOST_CHECK_CLOSE(object->poss[1][1], -1, 0.01);
	BOOST_CHECK_CLOSE(object->poss[1][2], -3.14, 0.01);
	BOOST_REQUIRE_EQUAL(object->nest.size(), 3);
	BOOST_REQUIRE_EQUAL(object->nest.at(0).size(), 2);
	BOOST_REQUIRE_EQUAL(object->nest.at(0).at(0).size(), 2);
	BOOST_REQUIRE_EQUAL(object->nest.at(0).at(1).size(), 3);
	BOOST_REQUIRE_EQUAL(object->nest.at(1).size(), 1);
	BOOST_REQUIRE_EQUAL(object->nest.at(1).at(0).size(), 1);
	BOOST_REQUIRE_EQUAL(object->nest.at(2).size(), 0);
	BOOST_REQUIRE(object->ptr);
	BOOST_CHECK_CLOSE(object->ptr->flt, 3.14, 0.01);
	BOOST_CHECK_EQUAL(object->ptr->str, "Lovely string");
}

BOOST_FIXTURE_TEST_CASE(LoadNestedObject, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/nested.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_CHECK_EQUAL(object->flt, 1.F);
	BOOST_CHECK_EQUAL(object->str, "one");
	BOOST_REQUIRE(object->ptr);
	BOOST_CHECK_EQUAL(object->ptr->flt, 2.F);
	BOOST_CHECK_EQUAL(object->ptr->str, "two");
	BOOST_REQUIRE(object->ptr->ptr);
	BOOST_CHECK_EQUAL(object->ptr->ptr->flt, 3.F);
	BOOST_CHECK_EQUAL(object->ptr->ptr->str, "three");
	BOOST_REQUIRE(object->ptr->ptr->ptr);
	BOOST_CHECK_EQUAL(object->ptr->ptr->ptr->flt, 4.F);
	BOOST_CHECK_EQUAL(object->ptr->ptr->ptr->str, "four");
	BOOST_REQUIRE(!object->ptr->ptr->ptr->ptr);
}

BOOST_FIXTURE_TEST_CASE(LoadImplicitObject, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/implicit.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_CHECK(object->ptr);
	BOOST_CHECK_EQUAL(object->flt, 1.F);
	BOOST_CHECK_EQUAL(object->ptr->str, "trigger");
	BOOST_CHECK_EQUAL(object->str, "after");
}

BOOST_FIXTURE_TEST_CASE(LoadEmptyObject, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/empty.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_CHECK_EQUAL(object->flt, 1.F);
	BOOST_CHECK(object->ptr);
	BOOST_CHECK_EQUAL(object->str, "after");
}

BOOST_DATA_TEST_CASE_F(JPP, various_parse_failures, fixturesIn(FIXTURESDIR "json/bad"), path)
{
	BOOST_CHECK_THROW(loadJson<std::unique_ptr<TestObject>>(path), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(LoadObjNoSuchType, JPP)
{
	BOOST_CHECK_THROW(loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/bad_type.json"), std::out_of_range);
}

BOOST_FIXTURE_TEST_CASE(LoadAbsObject, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/abs.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_REQUIRE(object->aptr);
	BOOST_CHECK_NO_THROW(object->aptr->dummy());
	BOOST_CHECK_EQUAL(object->aptr->base, "set base");
	auto subObject = dynamic_cast<SubObject *>(object->aptr.get());
	BOOST_REQUIRE(subObject);
	BOOST_CHECK_EQUAL(subObject->sub, "set sub");
}

BOOST_FIXTURE_TEST_CASE(LoadVectorPtr, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/vector_ptr.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_CHECK(object->str.empty());
	BOOST_CHECK_EQUAL(object->vptr.size(), 4);
	BOOST_CHECK_EQUAL(object->vptr.at(0)->str, "type");
	BOOST_CHECK_CLOSE(object->vptr.at(1)->flt, 3.14, .01);
	BOOST_CHECK(!object->vptr.at(2));
	BOOST_CHECK(object->vptr.at(3)->str.empty());
}

BOOST_FIXTURE_TEST_CASE(TestConversion, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/conv.json");
	BOOST_CHECK_EQUAL(object->postLoadCalled, 1);
	BOOST_REQUIRE(object);
	BOOST_CHECK_EQUAL(object->bl, true);
	BOOST_CHECK_EQUAL(object->flt, 3.14F);
}

BOOST_FIXTURE_TEST_CASE(LoadSharedObjectDiff, JPP)
{
	auto object = loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_diff.json");
	BOOST_CHECK(object->sptr);
	BOOST_CHECK(object->ssptr);
	BOOST_CHECK_NE(object->sptr, object->ssptr);
	BOOST_CHECK_EQUAL(object->sptr.use_count(), 1);
	BOOST_CHECK_EQUAL(object->ssptr.use_count(), 1);
}

BOOST_FIXTURE_TEST_CASE(LoadSharedObjectSame, JPP)
{
	auto object = loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_same.json");
	BOOST_CHECK(object->sptr);
	BOOST_CHECK(object->ssptr);
	BOOST_CHECK_EQUAL(object->sptr, object->ssptr);
	BOOST_CHECK_EQUAL(object->sptr.use_count(), 2);
	BOOST_CHECK_EQUAL(object->ssptr.use_count(), 2);
}

BOOST_FIXTURE_TEST_CASE(LoadSharedObjectDiffDefault, JPP)
{
	auto object = loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_diff_default.json");
	BOOST_CHECK(object->sptr);
	BOOST_CHECK(object->ssptr);
	BOOST_CHECK_NE(object->sptr, object->ssptr);
}

BOOST_FIXTURE_TEST_CASE(LoadSharedObjectWrongType, JPP)
{
	BOOST_CHECK_THROW(loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_wrong_type.json"),
			std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(LoadSharedObjectNull, JPP)
{
	auto object = loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_null.json");
	BOOST_CHECK(object->sptr);
	BOOST_CHECK(!object->ssptr);
}

using InputStringAndExpected = std::tuple<const char * const, std::string_view>;
auto const TEST_STRINGS = boost::unit_test::data::make<InputStringAndExpected>({
		{R"J("")J", ""},
		{R"J("non empty")J", "non empty"},
		{R"J("new\nline")J", "new\nline"},
		{R"J("quote\"mark")J", "quote\"mark"},
		{R"J("tab\t")J", "tab\t"},
		{R"J("back\bspace?")J", "back\bspace?"},
		{R"J("form\ffeed?")J", "form\ffeed?"},
		{R"J("a \u0007 bell")J", "a \a bell"},
});
auto const TEST_STRINGS_DECODE_ONLY = boost::unit_test::data::make<InputStringAndExpected>({
		{R"J("forward\/slash")J", "forward/slash"},
		{R"J("\u00a5 yen")J", "¥ yen"},
		{R"J("gbp \u00a3")J", "gbp £"},
		{R"J("\u007E tilde")J", "~ tilde"},
		{R"J("\u056b ARMENIAN SMALL LETTER INI")J", "ի ARMENIAN SMALL LETTER INI"},
		{R"J("\u0833 SAMARITAN PUNCTUATION BAU")J", "࠳ SAMARITAN PUNCTUATION BAU"},
});

BOOST_DATA_TEST_CASE(LoadStrings, TEST_STRINGS + TEST_STRINGS_DECODE_ONLY, input, exp)
{
	std::stringstream str {input};
	BOOST_CHECK_EQUAL(Persistence::JsonParsePersistence {}.loadState<std::string>(str), exp);
}

using CodePointAndString = std::tuple<unsigned long, std::string_view>;

BOOST_DATA_TEST_CASE(Utf8Decode,
		boost::unit_test::data::make<CodePointAndString>({
				{9, "\t"},
				{0x00010000, "𐀀"},
		}),
		codePoint, str)
{
	std::string out;
	BOOST_CHECK_NO_THROW(json::jsonParser::appendEscape(codePoint, out));
	BOOST_CHECK_EQUAL(out, str);
}

BOOST_DATA_TEST_CASE(Utf8DecodeBad, boost::unit_test::data::make<unsigned long>({0xd800, 0xdfff, 0x110000}), codePoint)
{
	std::string out;
	BOOST_CHECK_THROW(json::jsonParser::appendEscape(codePoint, out), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WriteTestNull)
{
	std::unique_ptr<TestObject> object {};
	std::stringstream outStream;
	Persistence::JsonWritePersistence {outStream}.saveState(object);
	BOOST_CHECK_EQUAL(outStream.view(), "null");
}

BOOST_AUTO_TEST_CASE(WriteTestDfl)
{
	auto object = std::make_unique<TestObject>();
	std::stringstream outStream;
	Persistence::JsonWritePersistence {outStream}.saveState(object);
	BOOST_CHECK_EQUAL(outStream.view(),
			R"({"p.typeid":"TestObject","flt":0,"str":"","bl":false,"pos":[0,0,0],"gpos":[0,0,0],"flts":[],"poss":[],"nest":[],"vptr":[]})");
}

BOOST_FIXTURE_TEST_CASE(WriteTestLoaded, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/load_object.json");
	std::stringstream outStream;
	Persistence::JsonWritePersistence {outStream}.saveState(object);
	BOOST_CHECK_EQUAL(outStream.view(),
			R"({"p.typeid":"TestObject","flt":3.14,"str":"Lovely string","bl":true,"pos":[3.14,6.28,1.57],"gpos":[2147483647,2147483646,-2147483648],"flts":[3.14,6.28,1.57,0,-1,-3.14],"poss":[[3.14,6.28,1.57],[0,-1,-3.14]],"nest":[[["a","b"],["c","d","e"]],[["f"]],[]],"ptr":{"p.typeid":"TestObject","flt":3.14,"str":"Lovely string","bl":false,"pos":[0,0,0],"gpos":[0,0,0],"flts":[],"poss":[],"nest":[],"vptr":[]},"vptr":[]})");
}

BOOST_FIXTURE_TEST_CASE(WriteTestLoadedAbs, JPP)
{
	auto object = loadJson<std::unique_ptr<TestObject>>(FIXTURESDIR "json/abs.json");
	std::stringstream outStream;
	Persistence::JsonWritePersistence {outStream}.saveState(object);
	BOOST_CHECK_EQUAL(outStream.view(),
			R"({"p.typeid":"TestObject","flt":0,"str":"","bl":false,"pos":[0,0,0],"gpos":[0,0,0],"flts":[],"poss":[],"nest":[],"aptr":{"p.typeid":"SubObject","base":"set base","sub":"set sub"},"vptr":[]})");
}

BOOST_FIXTURE_TEST_CASE(WriteTestLoadedShared, JPP)
{
	auto object = loadJson<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr_same.json");
	std::stringstream outStream;
	Persistence::seenSharedObjects.clear();
	Persistence::JsonWritePersistence {outStream}.saveState(object);
	BOOST_CHECK_EQUAL(Persistence::seenSharedObjects.size(), 1);
	BOOST_CHECK_EQUAL(outStream.view(),
			R"({"p.typeid":"SharedTestObject","sptr":{"p.typeid":"SubObject","p.id":"someid","base":"","sub":""},"ssptr":"someid"})");
}

BOOST_DATA_TEST_CASE(WriteSpecialStrings, TEST_STRINGS, exp, input)
{
	std::stringstream outStream;
	std::string copy(input);
	Persistence::JsonWritePersistence {outStream}.saveState(copy);
	BOOST_CHECK_EQUAL(outStream.view(), exp);
}

BOOST_AUTO_TEST_CASE(GetDefaultId)
{
	SubObject2 subObject;
	const auto subObjectId {subObject.getId()};

	BOOST_TEST_CONTEXT(subObjectId) {
		auto ptr = std::stoul(subObjectId, nullptr, 16);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		BOOST_CHECK_EQUAL(ptr, reinterpret_cast<decltype(ptr)>(&subObject));
	}
}
