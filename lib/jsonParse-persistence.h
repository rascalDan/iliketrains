#pragma once

#include "jsonParse.h" // IWYU pragma: export
#include "persistence.h" // IWYU pragma: export
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

namespace Persistence {
	class JsonParsePersistence : public json::jsonParser, ParseBase {
	public:
		template<typename T>
		inline T
		loadState(std::istream & in)
		{
			if (!in.good()) {
				throw std::runtime_error("Input stream not in good state");
			}
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
		void pushNumber(std::string_view value) override;
		void pushNull() override;
		void pushText(std::string && value) override;
		void pushKey(std::string && k) override;
		void endArray() override;
		void endObject() override;

		template<typename T> inline void pushValue(T && value);
		inline SelectionPtr & current();
	};

	class JsonWritePersistence : public Writer {
	public:
		explicit JsonWritePersistence(std::ostream & s);

		template<typename T>
		inline void
		saveState(T & t) const
		{
			SelectionT<T> {t}.write(*this);
		}

	protected:
		void beginObject() const override;
		void beginArray() const override;
		void pushValue(bool value) const override;
		void pushValue(float value) const override;
		void pushValue(int value) const override;
		void pushValue(std::nullptr_t) const override;
		void pushValue(const std::string_view value) const override;
		void nextValue() const override;
		void pushKey(const std::string_view k) const override;
		void endArray() const override;
		void endObject() const override;

	private:
		std::ostream & strm;
	};
}
