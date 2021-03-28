#ifndef JSONFLEXLEXER_H
#define JSONFLEXLEXER_H

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
		virtual void BeginObject() = 0;
		virtual void BeginArray() = 0;

		virtual void PushBoolean(bool) = 0;
		virtual void PushNumber(float) = 0;
		virtual void PushNull() = 0;
		virtual void PushText(std::string &&) = 0;
		virtual void PushKey(std::string &&) = 0;

		virtual void EndArray() = 0;
		virtual void EndObject() = 0;

		void LexerError(const char * msg) override;

		std::string buf;
	};
}

#endif
