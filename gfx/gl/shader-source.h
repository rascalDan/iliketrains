#ifndef SHADER_SOURCE_H
#define SHADER_SOURCE_H

#include <GL/glew.h>

struct GLsource {
	const GLchar * text;
	GLint len;
	GLuint type;
};

constexpr auto
constexpr_strlen(const GLchar * const s)
{
	auto e {s};
	while (*++e) { }
	return e - s;
}

#endif
