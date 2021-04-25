#ifndef JSONPARSE_PERSISTANCE
#define JSONPARSE_PERSISTANCE

#include "jsonParse.h" // IWYU pragma: export
#include "persistance.h" // IWYU pragma: export
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

namespace Persistanace {
	class JsonParsePersistance : public json::jsonParser {
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

		void BeginObject() override;
		void BeginArray() override;
		void PushBoolean(bool value) override;
		void PushNumber(float value) override;
		void PushNull() override;
		void PushText(std::string && value) override;
		void PushKey(std::string && k) override;
		void EndArray() override;
		void EndObject() override;

		Stack stk;

		template<typename T> inline void PushValue(T && value);
		inline SelectionPtr & current();
	};
}

#endif
