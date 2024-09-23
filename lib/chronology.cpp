#include "chronology.h"

time_t
operator""_time_t(const char * iso, size_t)
{
	struct tm tm {};

	if (const auto end = strptime(iso, "%FT%T", &tm); !end || *end) {
		throw std::invalid_argument("Invalid date");
	}
	return mktime(&tm);
}
