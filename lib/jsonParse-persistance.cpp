#include "jsonParse-persistance.h"

namespace Persistanace {
	void
	JsonParsePersistance::loadState(std::istream & in)
	{
		this->switch_streams(&in, nullptr);
		yy_push_state(0);
		yylex();
	}

	void
	JsonParsePersistance::BeginObject()
	{
		current()->beforeValue(stk);
		current()->BeginObject(stk);
	}

	void
	JsonParsePersistance::BeginArray()
	{
		current()->BeginArray(stk);
	}

	void
	JsonParsePersistance::PushBoolean(bool value)
	{
		PushValue(value);
	}

	void
	JsonParsePersistance::PushNumber(float value)
	{
		PushValue(value);
	}

	void
	JsonParsePersistance::PushNull()
	{
		PushValue(nullptr);
	}

	void
	JsonParsePersistance::PushText(std::string && value)
	{
		PushValue(value);
	}

	void
	JsonParsePersistance::PushKey(std::string && k)
	{
		stk.push(current()->select(k));
	}

	void
	JsonParsePersistance::EndArray()
	{
		stk.pop();
	}

	void
	JsonParsePersistance::EndObject()
	{
		current()->EndObject(stk);
		current()->EndObject(stk);
	}

	template<typename T>
	inline void
	JsonParsePersistance::PushValue(T && value)
	{
		current()->beforeValue(stk);
		(*current())(value);
		stk.pop();
	}

	inline SelectionPtr &
	JsonParsePersistance::current()
	{
		return stk.top();
	}
}
