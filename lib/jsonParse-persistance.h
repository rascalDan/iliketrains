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
		explicit JsonParsePersistance(std::istream & in);

		template<typename T>
		void
		loadState(T & t)
		{
			stk.push(std::make_unique<SelectionT<T>>(std::ref(t)));
			loadState();
		}

	protected:
		void loadState();

		void BeginObject() override;
		void BeginArray() override;
		void PushBoolean(bool value) override;
		void PushNumber(float value) override;
		void PushNull() override;
		void PushText(std::string && value) override;
		void PushKey(std::string && k) override;
		void EndArray() override;
		void EndObject() override;

	private:
		Stack stk;

		template<typename T> inline void PushValue(T && value);
		inline SelectionPtr & current();
	};
}

#endif
