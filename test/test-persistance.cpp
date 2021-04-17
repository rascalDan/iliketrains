#define BOOST_TEST_MODULE test_persistance

#include <boost/test/unit_test.hpp>

#include <functional>
#include <glm/glm.hpp>
#include <iosfwd>
#include <jsonParse.h>
#include <memory>
#include <persistance.h>
#include <string>
#include <vector>

struct TestObject;

namespace Persistanace {
	struct JsonLoadPersistanceStore : public json::jsonParser {
		explicit JsonLoadPersistanceStore(std::istream & in) : json::jsonParser {&in} { }

		Stack stk;

		inline SelectionPtr &
		current()
		{
			return stk.top();
		}

		template<typename T>
		void
		loadState(std::unique_ptr<T> & t)
		{
			stk.push(std::make_unique<SelectionT<std::unique_ptr<T>>>(std::ref(t)));
			yy_push_state(0);
			yylex();
		}
		void
		BeginObject() override
		{
			stk.push(current()->BeginObject());
		}
		void
		BeginArray() override
		{
			current()->BeginArray(stk);
		}
		template<typename T>
		inline void
		PushValue(T && value)
		{
			current()->beforeValue(stk);
			(*current())(value);
			stk.pop();
		}
		void
		PushBoolean(bool value) override
		{
			PushValue(value);
		}
		void
		PushNumber(float value) override
		{
			PushValue(value);
		}
		void
		PushNull() override
		{
			PushValue(nullptr);
		}
		void
		PushText(std::string && value) override
		{
			PushValue(value);
		}
		void
		PushKey(std::string && k) override
		{
			stk.push(current()->select(k));
		}
		void
		EndArray() override
		{
			stk.pop();
		}
		void
		EndObject() override
		{
			stk.pop();
		}
	};
}

#define STORE_TYPE store.persistType(typeid(*this))
#define STORE_MEMBER(mbr) store.persistValue(#mbr, mbr)

struct TestObject : public Persistanace::Persistable {
	TestObject() = default;

	float flt {};
	std::string str {};
	bool bl {};
	glm::vec3 pos {};
	std::vector<float> flts;
	std::vector<glm::vec3> poss;
	std::vector<std::vector<std::vector<std::string>>> nest;
	std::unique_ptr<TestObject> ptr;

	void
	persist(Persistanace::PersistanceStore & store) override
	{
		STORE_MEMBER(flt) && STORE_MEMBER(str) && STORE_MEMBER(bl) && STORE_MEMBER(pos) && STORE_MEMBER(flts)
				&& STORE_MEMBER(poss) && STORE_MEMBER(nest) && STORE_MEMBER(ptr);
	}
};

const std::string input(R"J({
	"@typeid": "TestObject",
	"flt": 3.14,
	"str": "Lovely string",
	"bl": true,
	"pos": [3.14, 6.28, 1.57],
	"flts": [3.14, 6.28, 1.57, 0, -1, -3.14],
	"poss": [[3.14, 6.28, 1.57], [0, -1, -3.14]],
	"nest": [[["a","b"],["c","d","e"]],[["f"]],[]],
	"ptr": {
		"@typeid": "TestObject",
		"flt": 3.14,
		"str": "Lovely string",
	},
})J");

BOOST_AUTO_TEST_CASE(load_object)
{
	Persistanace::Persistable::addFactory("TestObject", std::make_unique<TestObject>);
	std::stringstream ss {input};
	Persistanace::JsonLoadPersistanceStore jlps {ss};
	std::unique_ptr<TestObject> to;
	jlps.loadState(to);

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
