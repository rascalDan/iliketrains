#include "shader.h"
#include <array>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

Shader::ProgramHandle::ProgramHandle(std::initializer_list<GLuint> srcs) : viewProjection_uniform {}, model_uniform {}
{
	for (const auto & srcId : srcs) {
		glAttachShader(m_program, srcId);
	}

	glBindAttribLocation(m_program, 0, "position");
	glBindAttribLocation(m_program, 1, "texCoord");
	glBindAttribLocation(m_program, 2, "normal");

	glLinkProgram(m_program);
	CheckShaderError(m_program, GL_LINK_STATUS, true, "Error linking shader program");

	glValidateProgram(m_program);
	CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Invalid shader program");

	viewProjection_uniform = glGetUniformLocation(m_program, "viewProjection");
	model_uniform = glGetUniformLocation(m_program, "model");
}

Shader::Shader() :
	programs {{{
			Source {{basicShader_vs, basicShader_vs_len}, GL_VERTEX_SHADER}.id,
			Source {{basicShader_fs, basicShader_fs_len}, GL_FRAGMENT_SHADER}.id,
	}}}
{
}

void
Shader::setView(glm::mat4 proj) const
{
	for (const auto & prog : programs) {
		glUseProgram(prog.m_program);
		glUniformMatrix4fv(prog.viewProjection_uniform, 1, GL_FALSE, &proj[0][0]);
	}
}

void
Shader::setUniform(const GLchar * uniform, glm::vec3 v) const
{
	for (const auto & prog : programs) {
		if (auto loc = glGetUniformLocation(prog.m_program, uniform); loc >= 0) {
			glUseProgram(prog.m_program);
			glUniform3fv(loc, 1, &v[0]);
		}
	}
}

void
Shader::setModel(glm::mat4 model, Program pid) const
{
	auto & prog = programs[(int)pid];
	glUseProgram(prog.m_program);
	glUniformMatrix4fv(prog.model_uniform, 1, GL_FALSE, &model[0][0]);
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
