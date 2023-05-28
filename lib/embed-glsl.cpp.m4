changecom() dnl
// NAME
#include "gfx/gl/shader.h"
#include "lib/strlen.h"
#include "substr(TYPE,1)-NAME.h"
#include <glad/gl.h>

		constexpr const GLchar * src {R"GLSL-EMBED(dnl
include(SOURCE))GLSL-EMBED"};
constexpr auto len {constexpr_strlen(src)};

const Shader NAME`_'substr(TYPE,1) { src, len, GLTYPE };
