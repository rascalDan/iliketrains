#pragma once

#include <glRef.h>
#include <glad/gl.h>
#include <string_view>

class Shader {
public:
	using ShaderRef = glRef<GLuint, &glCreateShader, &glDeleteShader>;

	constexpr Shader(const GLchar * text, GLuint type) : text {text}, type {type} { }

	[[nodiscard]] ShaderRef compile() const;
	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);

private:
	const std::basic_string_view<GLchar> text;
	GLuint type;
};
