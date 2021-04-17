#include "jsonParse.h"
#include <cstdlib>
#include <stdexcept>
#include <string>

void
json::jsonParser::LexerError(const char * msg)
{
	throw std::runtime_error(msg);
}

void
json::jsonParser::appendEscape(const char * cphs, std::string & str)
{
	appendEscape(std::strtoul(cphs, nullptr, 16), str);
}

void
json::jsonParser::appendEscape(unsigned long cp, std::string & str)
{
	if (cp <= 0x7F) {
		str += (char)cp;
	}
	else if (cp <= 0x7FF) {
		str += char((cp >> 6) + 192);
		str += char((cp & 63) + 128);
	}
	else if (0xd800 <= cp && cp <= 0xdfff) {
		throw std::range_error("Invalid UTF-8 sequence");
	}
	else if (cp <= 0xFFFF) {
		str += char((cp >> 12) + 224);
		str += char(((cp >> 6) & 63) + 128);
		str += char((cp & 63) + 128);
	}
	else if (cp <= 0x10FFFF) {
		str += char((cp >> 18) + 240);
		str += char(((cp >> 12) & 63) + 128);
		str += char(((cp >> 6) & 63) + 128);
		str += char((cp & 63) + 128);
	}
}
