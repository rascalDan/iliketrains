changecom()dnl
// NAME
#include "substr(TYPE,1)-NAME.h"
#include <GL/glew.h>
#include "gfx/gl/glSource.h"
#include "lib/strings.hpp"

constexpr const GLchar * src { R"GLSL-EMBED(dnl
include(SOURCE))GLSL-EMBED" };
constexpr auto len { constexpr_strlen (src) };

const GLsource NAME`_'substr(TYPE,1) { src, len, GLTYPE };
