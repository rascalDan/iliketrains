#pragma once

#include <glRef.h>
#include <glad/gl.h>
#include <string_view>
#include <thirdparty/ctre/include/ctre.hpp>

class Shader {
public:
	using Source = std::basic_string_view<GLchar>;
	using ShaderRef = glRef<GLuint, &glCreateShader, &glDeleteShader>;

	constexpr Shader(const GLchar * text, GLuint type) :
		text {text}, type {type}, lookups {ctre::search<R"(\bGL_[A-Z_]+\b)">(this->text)}
	{
	}

	[[nodiscard]] ShaderRef compile() const;

	[[nodiscard]] static GLint getShaderParam(GLuint shader, GLenum pname);

private:
	void checkShaderError(GLuint shader) const;

	const Source text;
	GLuint type;
	bool lookups;
};
