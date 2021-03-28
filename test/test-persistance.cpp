#define BOOST_TEST_MODULE test_persistance

#include <boost/test/unit_test.hpp>

#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <jsonParse.h>
#include <maths.h>
#include <memory>
#include <stack>
#include <stream_support.hpp>
#include <utility>
#include <variant>
#include <vector>

struct PersistanceStore;
struct Selection;
using SelectionPtr = std::unique_ptr<Selection>;
struct Selection {
	virtual ~Selection() = default;
	virtual void
	operator()(float &)
	{
		throw std::runtime_error("Unexpected float");
	}
	virtual void
	operator()(bool &)
	{
		throw std::runtime_error("Unexpected bool");
	}
	virtual void
	operator()(const std::nullptr_t &)
	{
		throw std::runtime_error("Unexpected null");
	}
	virtual void
	operator()(std::string &)
	{
		throw std::runtime_error("Unexpected string");
	}
	virtual void
	BeginArray()
	{
		throw std::runtime_error("Unexpected array");
	}
	virtual SelectionPtr
	BeginObject()
	{
		throw std::runtime_error("Unexpected object");
	}
	virtual SelectionPtr select(std::string)
	{
		throw std::runtime_error("Unexpected persist");
	}
};

template<typename T> struct SelectionT : public Selection {
	SelectionT(T & value) : v {value} { }
	void
	operator()(T & evalue) override
	{
		std::swap(v, evalue);
	}
	T & v;
};

template<glm::length_t L, glm::qualifier Q> struct SelectionT<glm::vec<L, float, Q>> : public Selection {
	SelectionT(glm::vec3 & value) : v {value}
	{
		CLOG(__PRETTY_FUNCTION__);
	}
	void
	operator()(float & value) override
	{
		v[idx++] = value;
	}
	void
	BeginArray() override
	{
	}
	glm::vec3 & v;
	glm::vec3::length_type idx {0};
};

template<typename T> struct SelectionT<std::vector<T>> : public Selection {
	SelectionT(std::vector<T> & value) : v {value} { }
	void
	operator()(T & value) override
	{
		v.push_back(value);
	}
	void
	BeginArray()
	{
	}
	std::vector<T> & v;
};

struct TestObject;

template<typename Ptr> struct MakeObjectByTypeName : public Selection {
	MakeObjectByTypeName(Ptr & o) : o {o} { }
	virtual void
	operator()(std::string &)
	{
		o = std::make_unique<TestObject>(); // TODO shared_ptr
	}
	Ptr & o;
};

struct PersistanceStore {
	// virtual bool persistType(const std::type_info &) = 0;
	template<typename T>
	bool
	persistValue(const std::string_view & key, T & value)
	{
		if (key == name) {
			sel = std::make_unique<SelectionT<T>>(std::ref(value));
			return false;
		}
		return true;
	}
	const std::string & name;
	std::unique_ptr<Selection> sel {};
};

template<typename Ptr> struct SelectionObj : public Selection {
	SelectionObj(Ptr & o) : v {o} { }

	SelectionPtr
	select(std::string mbr) override
	{
		if (mbr == "@typeid") {
			return std::make_unique<MakeObjectByTypeName<Ptr>>(std::ref(v));
		}
		else if (v) {
			CLOG(mbr);
			PersistanceStore ps {mbr};
			v->persist(ps);
			return std::move(ps.sel);
		}
		throw std::runtime_error("cannot select member of null object");
	}

	Ptr & v;
};

template<typename T> struct SelectionT<std::unique_ptr<T>> : public Selection {
	SelectionT(std::unique_ptr<T> & o) : v {o} { }
	SelectionPtr
	BeginObject() override
	{
		CLOG(__PRETTY_FUNCTION__);
		return std::make_unique<SelectionObj<std::unique_ptr<T>>>(v);
	}
	std::unique_ptr<T> & v;
};

struct Persistable {
	virtual void persist(PersistanceStore & store) = 0;
};

struct JsonLoadPersistanceStore : public json::jsonParser {
	JsonLoadPersistanceStore(std::istream & in) : json::jsonParser {&in} { }

	std::stack<std::unique_ptr<Selection>> stk;
	std::unique_ptr<Selection> selection;

	template<typename T>
	void
	loadState(std::unique_ptr<T> & t)
	{
		yy_push_state(0);
		stk.push(std::make_unique<SelectionT<std::unique_ptr<T>>>(std::ref(t)));
		yylex();
	}
	void
	BeginObject() override
	{
		CLOG(__FUNCTION__);
		stk.push(stk.top()->BeginObject());
	}
	void
	BeginArray() override
	{
		CLOG(__FUNCTION__);
		selection->BeginArray();
	}
	void
	PushBoolean(bool value) override
	{
		CLOG(__FUNCTION__);
		(*selection)(value);
	}
	void
	PushNumber(float value) override
	{
		CLOG(__FUNCTION__);
		(*selection)(value);
	}
	void
	PushNull() override
	{
		CLOG(__FUNCTION__);
		(*selection)(nullptr);
	}
	void
	PushText(std::string && value) override
	{
		CLOG(__FUNCTION__);
		(*selection)(value);
	}
	void
	PushKey(std::string && k) override
	{
		CLOG(__FUNCTION__);
		selection = stk.top()->select(std::move(k));
	}
	void
	EndArray() override
	{
		CLOG(__FUNCTION__);
	}
	void
	EndObject() override
	{
		CLOG(__FUNCTION__);
		stk.pop();
	}
};

#define STORE_TYPE store.persistType(typeid(*this))
#define STORE_MEMBER(mbr) store.persistValue(#mbr##sv, mbr)

struct TestObject : public Persistable {
	TestObject() = default;

	float flt {};
	std::string str {};
	bool bl {};
	glm::vec3 pos {};
	std::vector<float> flts;

	void
	persist(PersistanceStore & store) override
	{
		using namespace std::literals;
		STORE_MEMBER(flt) && STORE_MEMBER(str) && STORE_MEMBER(bl) && STORE_MEMBER(pos) && STORE_MEMBER(flts);
	}
};

const std::string input(R"J({
	"@typeid": "TestObject",
	"flt": 3.14,
	"str": "Lovely string",
	"bl": true,
	"pos": [3.14, 6.28, 1.57],
	"flts": [3.14, 6.28, 1.57, 0, -1, -3.14],
})J");

BOOST_AUTO_TEST_CASE(load_object)
{
	std::stringstream ss {input};
	JsonLoadPersistanceStore jlps {ss};
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
}
