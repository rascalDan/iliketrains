changecom()dnl
// NAME
#include "substr(TYPE,1)-NAME.h"
#include <cstring>

constexpr const GLchar * src { R"GLSL-EMBED(dnl
include(SOURCE))GLSL-EMBED" };
constexpr auto len { constexpr_strlen (src) };

const GLsource NAME`_'substr(TYPE,1) { src, len, GLTYPE };