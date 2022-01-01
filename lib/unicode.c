#include "unicode.h"
#include <glib.h>

const char *
next_char(const char * c)
{
	return g_utf8_next_char(c);
}
uint32_t
get_codepoint(const char * c)
{
	return g_utf8_get_char(c);
}
