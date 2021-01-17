#include "shader.h"
#include "transform.h"
#include <glm/glm.hpp>
#include <res/fs-basicShader.h>
#include <res/vs-basicShader.h>
#include <stdexcept>

Shader::Shader(const std::string &) :
	m_program {glCreateProgram()}, m_shaders {CreateShader(
													  (GLchar *)(basicShader_vs), basicShader_vs_len, GL_VERTEX_SHADER),
										   CreateShader(
												   (GLchar *)basicShader_fs, basicShader_fs_len, GL_FRAGMENT_SHADER)},
	m_uniforms {}
{
	for (auto m_shader : m_shaders) {
		glAttachShader(m_program, m_shader);
	}

	glBindAttribLocation(m_program, 0, "position");
	glBindAttribLocation(m_program, 1, "texCoord");
	glBindAttribLocation(m_program, 2, "normal");

	glLinkProgram(m_program);
	CheckShaderError(m_program, GL_LINK_STATUS, true, "Error linking shader program");

	glValidateProgram(m_program);
	CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Invalid shader program");

	m_uniforms = {glGetUniformLocation(m_program, "MVP"), glGetUniformLocation(m_program, "Normal"),
			glGetUniformLocation(m_program, "lightDirection")};
}

Shader::~Shader()
{
	for (auto m_shader : m_shaders) {
		glDetachShader(m_program, m_shader);
		glDeleteShader(m_shader);
	}

	glDeleteProgram(m_program);
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

	glUniformMatrix4fv(m_uniforms[0], 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(m_uniforms[1], 1, GL_FALSE, &Normal[0][0]);
	glUniform3f(m_uniforms[2], 0.0F, 0.0F, 1.0F);
}

void
Shader::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string & errorMessage)
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

		throw std::runtime_error {errorMessage + ": '" + std::string {error.data(), error.size()} + "'"};
	}
}

GLuint
Shader::CreateShader(const GLchar * text, GLint len, unsigned int type)
{
	GLuint shader = glCreateShader(type);

	if (shader == 0) {
		throw std::runtime_error {"Error compiling shader type " + std::to_string(type)};
	}

	glShaderSource(shader, 1, &text, &len);
	glCompileShader(shader);

	CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error compiling shader!");

	return shader;
}
