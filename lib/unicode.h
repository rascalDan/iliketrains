#ifndef UNICODE_H
#define UNICODE_H

// Wrappers of some glib functions (why are we using glib then?) which we want, but glib.h is a bit C like

#ifdef __cplusplus
#	include <cstdint>
extern "C" {
#else
#	include <stdint.h>
#endif

const char * next_char(const char *);
uint32_t get_codepoint(const char *);

#ifdef __cplusplus
}
#endif

#endif
