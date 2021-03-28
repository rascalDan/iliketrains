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
	PushBoolean(true);
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{false} {
	PushBoolean(false);
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{number} {
	PushNumber(std::strtof(YYText(), NULL));
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL>{null} {
	PushNull();
	yy_pop_state();
}

<ARRAY_ITEM,INITIAL,OBJECT_ITEM,OBJECT_ITEM_OR_END>{beginstr} {
	yy_push_state(STRING);
}

<ARRAY_ITEM,INITIAL>{beginobj} {
	BeginObject();
	BEGIN(OBJECT_ITEM_OR_END);
}

<ARRAY_ITEM,INITIAL>{beginarray} {
	BeginArray();
	BEGIN(ARRAY_NEXT);
	yy_push_state(ARRAY_ITEM);
}

<STRING>{endstr} {
	yy_pop_state();
	switch (YY_START) {
		case ARRAY_ITEM:
		case INITIAL:
			PushText(std::move(buf));
			yy_pop_state();
			break;
		case OBJECT_ITEM:
		case OBJECT_ITEM_OR_END:
			PushKey(std::move(buf));
			BEGIN(COLON);
			break;
	}
	buf.clear();
}

<OBJECT_NEXT,OBJECT_ITEM_OR_END>{endobj} {
	EndObject();
	yy_pop_state();
}

<ARRAY_ITEM>{endarray} {
	EndArray();
	yy_pop_state();
	yy_pop_state();
}

<ARRAY_NEXT>{endarray} {
	EndArray();
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

%%

// Make iwyu think unistd.h is required
[[maybe_unused]]static auto x=getpid;
