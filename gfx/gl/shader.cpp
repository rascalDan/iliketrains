#include "shader.h"
#include <array>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

Shader::Shader() : viewProjection_uniform {}, model_uniform {}, lightDir_uniform {}
{
	glAttachShader(m_program, Source {{basicShader_vs, basicShader_vs_len}, GL_VERTEX_SHADER}.id);
	glAttachShader(m_program, Source {{basicShader_fs, basicShader_fs_len}, GL_FRAGMENT_SHADER}.id);

	glBindAttribLocation(m_program, 0, "position");
	glBindAttribLocation(m_program, 1, "texCoord");
	glBindAttribLocation(m_program, 2, "normal");

	glLinkProgram(m_program);
	CheckShaderError(m_program, GL_LINK_STATUS, true, "Error linking shader program");

	glValidateProgram(m_program);
	CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Invalid shader program");

	viewProjection_uniform = glGetUniformLocation(m_program, "viewProjection");
	model_uniform = glGetUniformLocation(m_program, "model");
	lightDir_uniform = glGetUniformLocation(m_program, "lightDirection");
}

void
Shader::Bind() const
{
	glUseProgram(m_program);
}

void
Shader::setView(glm::mat4 proj) const
{
	glUniformMatrix4fv(viewProjection_uniform, 1, GL_FALSE, &proj[0][0]);
	const glm::vec3 lightDir = glm::normalize(glm::vec3 {1, -1, 0});
	glUniform3fv(lightDir_uniform, 1, &lightDir[0]);
}

void
Shader::setModel(glm::mat4 model) const
{
	glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &model[0][0]);
}

void
Shader::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage)
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

Shader::Source::Source(const std::basic_string_view<unsigned char> text, GLuint type) :
	Source {(const GLchar *)(text.data()), (GLint)(text.length()), type}
{
}

Shader::Source::Source(const GLchar * text, GLint len, unsigned int type) : id {type}
{
	glShaderSource(id, 1, &text, &len);
	glCompileShader(id);

	CheckShaderError(id, GL_COMPILE_STATUS, false, "Error compiling shader!");
}
