#include "testHelpers.h"
#include <cstdarg>

std::unique_ptr<char, decltype(&free)>
uasprintf(const char * fmt, ...)
{
	char * buf {};
	va_list args;
	va_start(args, fmt);
	if (vasprintf(&buf, fmt, args) < 0) {
		va_end(args);
		return {nullptr, &free};
	}
	va_end(args);
	return std::unique_ptr<char, decltype(&free)> {buf, &free};
}
