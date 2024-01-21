changecom() dnl
// NAME
#include "gfx/gl/shader.h"
#include "substr(TYPE,1)-NAME.h"
#include <glad/gl.h>

constexpr Shader NAME`_'substr(TYPE,1) {
	R"GLSL-EMBED(dnl
	include(SOURCE))GLSL-EMBED", GLTYPE };
