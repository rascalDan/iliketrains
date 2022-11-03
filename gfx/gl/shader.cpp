#include "shader.h"
#include <array>
#include <stdexcept>
#include <string>

Shader::operator GLuint() const
{
	if (!shader) {
		shader.emplace(type);
		glShaderSource(*shader, 1, &text, &len);
		glCompileShader(*shader);

		CheckShaderError(*shader, GL_COMPILE_STATUS, false, "Error compiling shader!");
	}
	return *shader;
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
