#pragma once

#ifndef FLEX_SCANNER
#	define yyFlexLexer jsonBaseFlexLexer
#	include <FlexLexer.h>
#endif
#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

namespace json {
	class jsonParser : public yyFlexLexer {
	public:
		using yyFlexLexer::yyFlexLexer;
		int yylex() override;

		static void appendEscape(const char *, std::string &);
		static void appendEscape(unsigned long, std::string &);

	protected:
		virtual void beginObject() = 0;
		virtual void beginArray() = 0;

		virtual void pushBoolean(bool) = 0;
		virtual void pushNumber(std::string_view) = 0;
		virtual void pushNull() = 0;
		virtual void pushText(std::string &&) = 0;
		virtual void pushKey(std::string &&) = 0;

		virtual void endArray() = 0;
		virtual void endObject() = 0;

		void LexerError(const char * msg) override;

		std::string buf;
	};
}
