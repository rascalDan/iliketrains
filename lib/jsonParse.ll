%option batch
%option c++
%option noyywrap
%option 8bit
%option stack
%option yylineno
%option yyclass="json::jsonParser"
%option prefix="jsonBase"

%{
#include <string>
#include <utility>
class jsonBaseFlexLexer;
#include "jsonParse.h"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#ifdef __clang__
#pragma clang diagnostic ignored "-Wnull-conversion"
#endif
%}

beginobj "{"
endobj "}"
beginarray "["
endarray "]"
beginstr "\""
endstr "\""
true "true"
false "false"
null "null"
number [-+]?[0-9]+(\.[0-9]+)?
colon ":"
separator ","
escape "\\"
text [^\\\"]*

%x OBJECT_ITEM
%x OBJECT_ITEM_OR_END
%x OBJECT_NEXT
%x ARRAY_ITEM
%x ARRAY_NEXT
%x COLON
%x TEXT
%x STRING
%x ESCAPE

%%

<ARRAY_ITEM,INITIAL>{true} {
	pushBoolean(true);
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{false} {
	pushBoolean(false);
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{number} {
	pushNumber(std::strtof(YYText(), NULL));
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{null} {
	pushNull();
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL,OBJECT_ITEM,OBJECT_ITEM_OR_END>{beginstr} {
	yy_push_state(STRING);
}

<ARRAY_ITEM,INITIAL>{beginobj} {
	beginObject();
	BEGIN(OBJECT_ITEM_OR_END);
}

<ARRAY_ITEM,INITIAL>{beginarray} {
	beginArray();
	BEGIN(ARRAY_NEXT);
	yy_push_state(ARRAY_ITEM);
}

<STRING>{endstr} {
	yy_pop_state();
	switch (YY_START) {
		case ARRAY_ITEM:
		case INITIAL:
			pushText(std::move(buf));
			yy_pop_state();
			break;
		case OBJECT_ITEM:
		case OBJECT_ITEM_OR_END:
			pushKey(std::move(buf));
			BEGIN(COLON);
			break;
	}
	buf.clear();
}

<OBJECT_NEXT,OBJECT_ITEM_OR_END>{endobj} {
	endObject();
	yy_pop_state();
}

<ARRAY_ITEM>{endarray} {
	endArray();
	yy_pop_state();
	yy_pop_state();
}

<ARRAY_NEXT>{endarray} {
	endArray();
	yy_pop_state();
}

<COLON>{colon} {
	BEGIN(OBJECT_NEXT);
	yy_push_state(INITIAL);
}

<OBJECT_NEXT>{separator} {
	BEGIN(OBJECT_ITEM);
}

<ARRAY_NEXT>{separator} {
	yy_push_state(INITIAL);
}

<STRING>{escape} {
	yy_push_state(ESCAPE);
}

<ESCAPE>"\"" { buf += "\""; yy_pop_state(); }
<ESCAPE>"\\" { buf += "\\"; yy_pop_state(); }
<ESCAPE>"/" { buf += "/"; yy_pop_state(); }
<ESCAPE>"b" { buf += "\b"; yy_pop_state(); }
<ESCAPE>"f" { buf += "\f"; yy_pop_state(); }
<ESCAPE>"n" { buf += "\n"; yy_pop_state(); }
<ESCAPE>"r" { buf += "\r"; yy_pop_state(); }
<ESCAPE>"t" { buf += "\t"; yy_pop_state(); }

<ESCAPE>"u"[0-9a-fA-Z]{4} {
  appendEscape(YYText(), buf);
	yy_pop_state();
}

<STRING>{text} {
	buf += YYText();
}

<*>[ \t\r\n\f] {
}

<*>. {
  LexerError("Unexpected input");
  // Make iwyu think unistd.h is required
  [[maybe_unused]]static constexpr auto x=getpid;
}
