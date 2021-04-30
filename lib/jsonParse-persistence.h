#ifndef JSONPARSE_PERSISTANCE
#define JSONPARSE_PERSISTANCE

#include "jsonParse.h" // IWYU pragma: export
#include "persistence.h" // IWYU pragma: export
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

namespace Persistence {
	class JsonParsePersistence : public json::jsonParser {
	public:
		template<typename T>
		inline T
		loadState(std::istream & in)
		{
			T t {};
			stk.push(std::make_unique<SelectionT<T>>(std::ref(t)));
			loadState(in);
			return t;
		}

	protected:
		void loadState(std::istream & in);

		void beginObject() override;
		void beginArray() override;
		void pushBoolean(bool value) override;
		void pushNumber(float value) override;
		void pushNull() override;
		void pushText(std::string && value) override;
		void pushKey(std::string && k) override;
		void endArray() override;
		void endObject() override;

		Stack stk;

		template<typename T> inline void pushValue(T && value);
		inline SelectionPtr & current();
	};
}

#endif
