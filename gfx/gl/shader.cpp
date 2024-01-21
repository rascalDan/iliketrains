#include "shader.h"
#include <array>
#include <stdexcept>
#include <string>

Shader::ShaderRef
Shader::compile() const
{
	ShaderRef shader {type};
	auto source = [&shader](auto text, GLint len) {
		glShaderSource(shader, 1, &text, &len);
	};
	source(text.data(), static_cast<GLint>(text.length()));
	glCompileShader(shader);

	CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error compiling shader!");
	return shader;
}

void
Shader::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage)
{
	GLint success = 0;

	if (isProgram) {
		glGetProgramiv(shader, flag, &success);
	}
	else {
		glGetShaderiv(shader, flag, &success);
	}

	if (success == GL_FALSE) {
		std::array<GLchar, 1024> error {};
		if (isProgram) {
			glGetProgramInfoLog(shader, error.size(), nullptr, error.data());
		}
		else {
			glGetShaderInfoLog(shader, error.size(), nullptr, error.data());
		}

		throw std::runtime_error {std::string {errorMessage} + ": '" + std::string {error.data(), error.size()} + "'"};
	}
}
