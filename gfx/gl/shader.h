#pragma once

#include <GL/glew.h>
#include <glRef.hpp>
#include <optional>
#include <string_view>

class Shader {
public:
	constexpr Shader(const GLchar * text, GLint len, GLuint type) : text {text}, len {len}, type {type} { }

	operator GLuint() const;
	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);

private:
	using ShaderRef = glRef<GLuint, &__glewCreateShader, &__glewDeleteShader>;
	const GLchar * text;
	GLint len;
	GLuint type;
	mutable std::optional<ShaderRef> shader {};
};
