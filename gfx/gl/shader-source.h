#ifndef SHADER_SOURCE_H
#define SHADER_SOURCE_H

#include <GL/glew.h>
#include <glRef.hpp>

struct GLsource {
	using ShaderRef = glRef<GLuint, __glewCreateShader, __glewDeleteShader>;

	const GLchar * text;
	GLint len;
	GLuint type;

	[[nodiscard]] ShaderRef compile() const;
};

constexpr auto
constexpr_strlen(const GLchar * const s)
{
	std::size_t ch {};
	while (s[ch]) {
		ch++;
	}
	return ch;
}

#endif
