#include "glSource.h"
#include <array>
#include <stdexcept>
#include <string>

GLsource::ShaderRef
GLsource::compile() const
{
	ShaderRef id {type};
	glShaderSource(id, 1, &text, &len);
	glCompileShader(id);

	CheckShaderError(id, GL_COMPILE_STATUS, false, "Error compiling shader!");
	return id;
}

void
GLsource::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage)
{
	GLint success = 0;
	std::array<GLchar, 1024> error {};

	if (isProgram) {
		glGetProgramiv(shader, flag, &success);
	}
	else {
		glGetShaderiv(shader, flag, &success);
	}

	if (success == GL_FALSE) {
		if (isProgram) {
			glGetProgramInfoLog(shader, error.size(), nullptr, error.data());
		}
		else {
			glGetShaderInfoLog(shader, error.size(), nullptr, error.data());
		}

		throw std::runtime_error {std::string {errorMessage} + ": '" + std::string {error.data(), error.size()} + "'"};
	}
}
