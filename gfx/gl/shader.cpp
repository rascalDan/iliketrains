#include "shader.h"
#include <array>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/fs-landmassShader.h>
#include <gfx/gl/shaders/fs-waterShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <gfx/gl/shaders/vs-landmassShader.h>
#include <gfx/gl/shaders/vs-waterShader.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

Shader::ProgramHandle::ProgramHandle(GLuint vs, GLuint fs) : viewProjection_uniform {}, model_uniform {}
{
	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);

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
					   Source {basicShader_vs}.id,
					   Source {basicShader_fs}.id,
			   },
			{
					Source {waterShader_vs}.id,
					Source {waterShader_fs}.id,
			},
			{
					Source {landmassShader_vs}.id,
					Source {landmassShader_fs}.id,
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

Shader::Source::Source(const GLsource src) : id {src.type}
{
	glShaderSource(id, 1, &src.text, &src.len);
	glCompileShader(id);

	CheckShaderError(id, GL_COMPILE_STATUS, false, "Error compiling shader!");
}
