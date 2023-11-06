#pragma once

#include <glRef.h>
#include <glad/gl.h>
#include <optional>
#include <string_view>

class Shader {
public:
	using ShaderRef = glRef<GLuint, &glCreateShader, &glDeleteShader>;

	constexpr Shader(const GLchar * text, GLint len, GLuint type) : text {text}, len {len}, type {type} { }

	[[nodiscard]] ShaderRef compile() const;
	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);

private:
	const GLchar * text;
	GLint len;
	GLuint type;
};
