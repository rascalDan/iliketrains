#include "jsonParse-persistence.h"

namespace Persistence {
	void
	JsonParsePersistence::loadState(std::istream & in)
	{
		this->switch_streams(&in, nullptr);
		yy_push_state(0);
		yylex();
	}

	void
	JsonParsePersistence::beginObject()
	{
		current()->beforeValue(stk);
		current()->beginObject(stk);
	}

	void
	JsonParsePersistence::beginArray()
	{
		current()->beforeValue(stk);
		current()->beginArray(stk);
	}

	void
	JsonParsePersistence::pushBoolean(bool value)
	{
		pushValue(value);
	}

	void
	JsonParsePersistence::pushNumber(float value)
	{
		pushValue(value);
	}

	void
	JsonParsePersistence::pushNull()
	{
		pushValue(nullptr);
	}

	void
	JsonParsePersistence::pushText(std::string && value)
	{
		pushValue(std::move(value));
	}

	void
	JsonParsePersistence::pushKey(std::string && k)
	{
		stk.push(current()->select(k));
	}

	void
	JsonParsePersistence::endArray()
	{
		stk.pop();
		stk.pop();
	}

	void
	JsonParsePersistence::endObject()
	{
		current()->endObject(stk);
		current()->endObject(stk);
	}

	template<typename T>
	inline void
	JsonParsePersistence::pushValue(T && value)
	{
		current()->beforeValue(stk);
		current()->setValue(std::forward<T>(value));
		stk.pop();
	}

	inline SelectionPtr &
	JsonParsePersistence::current()
	{
		return stk.top();
	}
}
