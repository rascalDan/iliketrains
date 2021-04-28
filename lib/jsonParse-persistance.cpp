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
	JsonParsePersistance::beginObject()
	{
		current()->beforeValue(stk);
		current()->beginObject(stk);
	}

	void
	JsonParsePersistance::beginArray()
	{
		current()->beforeValue(stk);
		current()->beginArray(stk);
	}

	void
	JsonParsePersistance::pushBoolean(bool value)
	{
		pushValue(value);
	}

	void
	JsonParsePersistance::pushNumber(float value)
	{
		pushValue(value);
	}

	void
	JsonParsePersistance::pushNull()
	{
		pushValue(nullptr);
	}

	void
	JsonParsePersistance::pushText(std::string && value)
	{
		pushValue(value);
	}

	void
	JsonParsePersistance::pushKey(std::string && k)
	{
		stk.push(current()->select(k));
	}

	void
	JsonParsePersistance::endArray()
	{
		stk.pop();
		stk.pop();
	}

	void
	JsonParsePersistance::endObject()
	{
		current()->endObject(stk);
		current()->endObject(stk);
	}

	template<typename T>
	inline void
	JsonParsePersistance::pushValue(T && value)
	{
		current()->beforeValue(stk);
		current()->setValue(value);
		stk.pop();
	}

	inline SelectionPtr &
	JsonParsePersistance::current()
	{
		return stk.top();
	}
}
