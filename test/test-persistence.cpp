#define BOOST_TEST_MODULE test_persistence

#include <boost/test/unit_test.hpp>

#include <glm/glm.hpp>
#include <iosfwd>
#include <jsonParse-persistence.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct AbsObject : public Persistence::Persistable {
	std::string base;

	bool
	persist(Persistence::PersistenceStore & store) override
	{
		return STORE_TYPE && STORE_MEMBER(base);
	}

	virtual void dummy() const = 0;
};

struct SubObject : public AbsObject {
	std::string sub;

	bool
	persist(Persistence::PersistenceStore & store) override
	{
		return AbsObject::persist(store) && STORE_TYPE && STORE_MEMBER(sub);
	}

	void
	dummy() const override
	{
	}
};

struct TestObject : public Persistence::Persistable {
	TestObject() = default;

	float flt {};
	std::string str {};
	bool bl {};
	glm::vec3 pos {};
	std::vector<float> flts;
	std::vector<glm::vec3> poss;
	std::vector<std::vector<std::vector<std::string>>> nest;
	std::unique_ptr<TestObject> ptr;
	std::unique_ptr<AbsObject> aptr;
	std::vector<std::unique_ptr<TestObject>> vptr;

	bool
	persist(Persistence::PersistenceStore & store) override
	{
		return STORE_TYPE && STORE_MEMBER(flt) && STORE_MEMBER(str) && STORE_MEMBER(bl) && STORE_MEMBER(pos)
				&& STORE_MEMBER(flts) && STORE_MEMBER(poss) && STORE_MEMBER(nest) && STORE_MEMBER(ptr)
				&& STORE_MEMBER(aptr) && STORE_MEMBER(vptr);
	}
};

struct JPP : public Persistence::JsonParsePersistence {
	template<typename T>
	T
	load_json(const char * path)
	{
		BOOST_TEST_CONTEXT(path) {
			std::ifstream ss {path};
			auto to = loadState<T>(ss);
			Persistence::sharedObjects.clear();
			BOOST_CHECK(stk.empty());
			BOOST_REQUIRE(to);
			return to;
		}
		// Presumably BOOST_TEST_CONTEXT is implemented as an if (...) { }
		throw std::logic_error("We shouldn't ever get here, but apparently we can!");
	}
};

BOOST_FIXTURE_TEST_CASE(load_object, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/load_object.json");
	BOOST_CHECK_CLOSE(to->flt, 3.14, 0.01);
	BOOST_CHECK_EQUAL(to->str, "Lovely string");
	BOOST_CHECK_EQUAL(to->bl, true);
	BOOST_CHECK_CLOSE(to->pos[0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(to->pos[1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(to->pos[2], 1.57, 0.01);
	BOOST_REQUIRE_EQUAL(to->flts.size(), 6);
	BOOST_CHECK_CLOSE(to->flts[0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(to->flts[1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(to->flts[2], 1.57, 0.01);
	BOOST_CHECK_CLOSE(to->flts[3], 0, 0.01);
	BOOST_CHECK_CLOSE(to->flts[4], -1, 0.01);
	BOOST_CHECK_CLOSE(to->flts[5], -3.14, 0.01);
	BOOST_REQUIRE_EQUAL(to->poss.size(), 2);
	BOOST_CHECK_CLOSE(to->poss[0][0], 3.14, 0.01);
	BOOST_CHECK_CLOSE(to->poss[0][1], 6.28, 0.01);
	BOOST_CHECK_CLOSE(to->poss[0][2], 1.57, 0.01);
	BOOST_CHECK_CLOSE(to->poss[1][0], 0, 0.01);
	BOOST_CHECK_CLOSE(to->poss[1][1], -1, 0.01);
	BOOST_CHECK_CLOSE(to->poss[1][2], -3.14, 0.01);
	BOOST_REQUIRE_EQUAL(to->nest.size(), 3);
	BOOST_REQUIRE_EQUAL(to->nest.at(0).size(), 2);
	BOOST_REQUIRE_EQUAL(to->nest.at(0).at(0).size(), 2);
	BOOST_REQUIRE_EQUAL(to->nest.at(0).at(1).size(), 3);
	BOOST_REQUIRE_EQUAL(to->nest.at(1).size(), 1);
	BOOST_REQUIRE_EQUAL(to->nest.at(1).at(0).size(), 1);
	BOOST_REQUIRE_EQUAL(to->nest.at(2).size(), 0);
	BOOST_REQUIRE(to->ptr);
	BOOST_CHECK_CLOSE(to->ptr->flt, 3.14, 0.01);
	BOOST_CHECK_EQUAL(to->ptr->str, "Lovely string");
}

BOOST_FIXTURE_TEST_CASE(load_nested_object, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/nested.json");
	BOOST_CHECK_EQUAL(to->flt, 1.F);
	BOOST_CHECK_EQUAL(to->str, "one");
	BOOST_REQUIRE(to->ptr);
	BOOST_CHECK_EQUAL(to->ptr->flt, 2.F);
	BOOST_CHECK_EQUAL(to->ptr->str, "two");
	BOOST_REQUIRE(to->ptr->ptr);
	BOOST_CHECK_EQUAL(to->ptr->ptr->flt, 3.F);
	BOOST_CHECK_EQUAL(to->ptr->ptr->str, "three");
	BOOST_REQUIRE(to->ptr->ptr->ptr);
	BOOST_CHECK_EQUAL(to->ptr->ptr->ptr->flt, 4.F);
	BOOST_CHECK_EQUAL(to->ptr->ptr->ptr->str, "four");
	BOOST_REQUIRE(!to->ptr->ptr->ptr->ptr);
}

BOOST_FIXTURE_TEST_CASE(load_implicit_object, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/implicit.json");
	BOOST_CHECK(to->ptr);
	BOOST_CHECK_EQUAL(to->flt, 1.F);
	BOOST_CHECK_EQUAL(to->ptr->str, "trigger");
	BOOST_CHECK_EQUAL(to->str, "after");
}

BOOST_FIXTURE_TEST_CASE(load_empty_object, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/empty.json");
	BOOST_CHECK_EQUAL(to->flt, 1.F);
	BOOST_CHECK(to->ptr);
	BOOST_CHECK_EQUAL(to->str, "after");
}

BOOST_FIXTURE_TEST_CASE(fail_implicit_abs_object, JPP)
{
	BOOST_CHECK_THROW(load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/implicit_abs.json"), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(fail_empty_abs_object, JPP)
{
	BOOST_CHECK_THROW(load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/empty_abs.json"), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(load_abs_object, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/abs.json");
	BOOST_REQUIRE(to->aptr);
	BOOST_CHECK_NO_THROW(to->aptr->dummy());
	BOOST_CHECK_EQUAL(to->aptr->base, "set base");
	auto s = dynamic_cast<SubObject *>(to->aptr.get());
	BOOST_REQUIRE(s);
	BOOST_CHECK_EQUAL(s->sub, "set sub");
}

BOOST_FIXTURE_TEST_CASE(load_vector_ptr, JPP)
{
	auto to = load_json<std::unique_ptr<TestObject>>(FIXTURESDIR "json/vector_ptr.json");
	BOOST_CHECK(to->str.empty());
	BOOST_CHECK_EQUAL(to->vptr.size(), 4);
	BOOST_CHECK_EQUAL(to->vptr.at(0)->str, "type");
	BOOST_CHECK_CLOSE(to->vptr.at(1)->flt, 3.14, .01);
	BOOST_CHECK(!to->vptr.at(2));
	BOOST_CHECK(to->vptr.at(3)->str.empty());
}

struct SharedTestObject : public Persistence::Persistable {
	SharedTestObject() = default;

	std::shared_ptr<AbsObject> sptr;
	std::shared_ptr<SubObject> ssptr;

	bool
	persist(Persistence::PersistenceStore & store) override
	{
		return STORE_TYPE && STORE_MEMBER(sptr) && STORE_MEMBER(ssptr);
	}
};

BOOST_FIXTURE_TEST_CASE(load_shared_object1, JPP)
{
	auto to = load_json<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr1.json");
	BOOST_CHECK(to->sptr);
	BOOST_CHECK(to->ssptr);
	BOOST_CHECK_NE(to->sptr, to->ssptr);
	BOOST_CHECK_EQUAL(to->sptr.use_count(), 1);
	BOOST_CHECK_EQUAL(to->ssptr.use_count(), 1);
}

BOOST_FIXTURE_TEST_CASE(load_shared_object2, JPP)
{
	auto to = load_json<std::unique_ptr<SharedTestObject>>(FIXTURESDIR "json/shared_ptr2.json");
	BOOST_CHECK(to->sptr);
	BOOST_CHECK(to->ssptr);
	BOOST_CHECK_EQUAL(to->sptr, to->ssptr);
	BOOST_CHECK_EQUAL(to->sptr.use_count(), 2);
	BOOST_CHECK_EQUAL(to->ssptr.use_count(), 2);
}
