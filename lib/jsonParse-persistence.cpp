#include "jsonParse-persistence.h"
#include <algorithm>
#include <array>
#include <iomanip>
#include <ostream>
#include <type_traits>
#include <utility>

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

	static inline void
	wrv(std::ostream & strm, char ch)
	{
		strm.put(ch);
	};
	static inline void
	wrh(std::ostream & strm, char ch)
	{
		using namespace std::literals;
		strm << R"(\u)"sv << std::setw(4) << std::hex << static_cast<int>(ch) << std::setw(1);
	}
	static inline void
	wre(std::ostream & strm, char e)
	{
		strm << '\\' << e;
	}
	template<char E>
	static inline void
	wre(std::ostream & strm, char)
	{
		wre(strm, E);
	}

	using OutFunc = void (*)(std::ostream &, char);
	using OutFuncs = std::array<OutFunc, 255>;
	static constexpr OutFuncs outFuncs {[]() {
		OutFuncs outFuncs;
		outFuncs.fill(&wrv);
		for (auto x = 0U; x < 0x20U; x += 1) {
			outFuncs[x] = &wrh;
		}
		outFuncs['\"'] = &wre<'"'>;
		outFuncs['\\'] = &wre<'\\'>;
		outFuncs['\b'] = &wre<'b'>;
		outFuncs['\f'] = &wre<'f'>;
		outFuncs['\n'] = &wre<'n'>;
		outFuncs['\r'] = &wre<'r'>;
		outFuncs['\t'] = &wre<'t'>;
		return outFuncs;
	}()};

	JsonWritePersistence::JsonWritePersistence(std::ostream & s) : strm {s}
	{
		strm << std::boolalpha // for Boolean
			 << std::defaultfloat // for Number
			 << std::setfill('0'); // for String \uNNNN
	}

	void
	JsonWritePersistence::beginObject() const
	{
		strm << '{';
	}

	void
	JsonWritePersistence::beginArray() const
	{
		strm << '[';
	}

	void
	JsonWritePersistence::pushValue(bool value) const
	{
		strm << value;
	}

	void
	JsonWritePersistence::pushValue(float value) const
	{
		strm << value;
	}

	void
	JsonWritePersistence::pushValue(std::nullptr_t) const
	{
		strm << "null";
	}

	void
	JsonWritePersistence::pushValue(const std::string_view value) const
	{
		strm << '"';
		std::for_each(value.begin(), value.end(), [this](char ch) {
			outFuncs[static_cast<unsigned char>(ch)](strm, ch);
		});
		strm << '"';
	}

	void
	JsonWritePersistence::nextValue() const
	{
		strm << ',';
	}

	void
	JsonWritePersistence::pushKey(const std::string_view k) const
	{
		pushValue(k);
		strm << ':';
	}

	void
	JsonWritePersistence::endArray() const
	{
		strm << ']';
	}

	void
	JsonWritePersistence::endObject() const
	{
		strm << '}';
	}
}
