#pragma once

#include <glRef.h>
#include <glad/gl.h>
#include <string_view>
#include <thirdparty/ctre/include/ctre.hpp>

class Shader {
public:
	using ShaderRef = glRef<GLuint, &glCreateShader, &glDeleteShader>;

	constexpr Shader(const GLchar * text, GLuint type) :
		text {text}, type {type}, lookups {ctre::search<R"(\bGL_[A-Z_]+\b)">(this->text)}
	{
	}

	[[nodiscard]] ShaderRef compile() const;

private:
	using Source = std::basic_string_view<GLchar>;

	void checkShaderError(GLuint shader, GLuint flag, std::string_view errorMessage) const;
	const Source text;
	GLuint type;
	bool lookups;
};
