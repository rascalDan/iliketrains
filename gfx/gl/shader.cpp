#include "shader.h"
#include "transform.h"
#include <array>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

Shader::Shader() : mvp_uniform {}, normal_uniform {}, lightDir_uniform {}
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

	mvp_uniform = glGetUniformLocation(m_program, "MVP");
	normal_uniform = glGetUniformLocation(m_program, "Normal");
	lightDir_uniform = glGetUniformLocation(m_program, "lightDirection");
}

void
Shader::Bind() const
{
	glUseProgram(m_program);
}

void
Shader::Update(const Transform & transform, const Camera & camera) const
{
	glm::mat4 MVP = transform.GetMVP(camera);
	glm::mat4 Normal = transform.GetModel();

	glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(normal_uniform, 1, GL_FALSE, &Normal[0][0]);
	glUniform3f(lightDir_uniform, 0.0F, 0.0F, 1.0F);
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
