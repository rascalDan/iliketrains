#include "jsonParse.h"

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
		str += cp;
	}
	else if (cp <= 0x7FF) {
		str += (cp >> 6) + 192;
		str += (cp & 63) + 128;
	}
	else if (0xd800 <= cp && cp <= 0xdfff) {
		throw std::range_error("Invalid UTF-8 sequence");
	}
	else if (cp <= 0xFFFF) {
		str += (cp >> 12) + 224;
		str += ((cp >> 6) & 63) + 128;
		str += (cp & 63) + 128;
	}
	else if (cp <= 0x10FFFF) {
		str += (cp >> 18) + 240;
		str += ((cp >> 12) & 63) + 128;
		str += ((cp >> 6) & 63) + 128;
		str += (cp & 63) + 128;
	}
}
