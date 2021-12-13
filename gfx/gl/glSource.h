#ifndef SHADER_SOURCE_H
#define SHADER_SOURCE_H

#include <GL/glew.h>
#include <glRef.hpp>
#include <string_view>

struct GLsource {
	using ShaderRef = glRef<GLuint, __glewCreateShader, __glewDeleteShader>;

	const GLchar * text;
	GLint len;
	GLuint type;

	[[nodiscard]] ShaderRef compile() const;
	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);
};

#endif
