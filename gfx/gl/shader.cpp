#include "shader.h"
#include "gfx/gl/glSource.h"
#include <array>
#include <cstddef>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/fs-landmassShader.h>
#include <gfx/gl/shaders/fs-waterShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <gfx/gl/shaders/vs-landmassShader.h>
#include <gfx/gl/shaders/vs-waterShader.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.hpp>
#include <maths.h>
#include <stdexcept>
#include <string>

Shader::ProgramHandle::ProgramHandle(GLuint vs, GLuint fs) : ProgramHandleBase {vs, fs}
{
	glBindAttribLocation(m_program, 0, "position");
	glBindAttribLocation(m_program, 1, "texCoord");
	glBindAttribLocation(m_program, 2, "normal");

	viewProjection_uniform = glGetUniformLocation(m_program, "viewProjection");
	model_uniform = glGetUniformLocation(m_program, "model");
}

Shader::Shader() :
	programs {{{
					   basicShader_vs.compile(),
					   basicShader_fs.compile(),
			   },
			{
					waterShader_vs.compile(),
					waterShader_fs.compile(),
			},
			{
					landmassShader_vs.compile(),
					landmassShader_fs.compile(),
			},
			{
					landmassShader_vs.compile(),
					basicShader_fs.compile(),
			}}}
{
}

void
Shader::setView(glm::mat4 proj) const
{
	for (const auto & prog : programs) {
		glUseProgram(prog.m_program);
		glUniformMatrix4fv(prog.viewProjection_uniform, 1, GL_FALSE, glm::value_ptr(proj));
	}
}

void
Shader::setUniform(const GLchar * uniform, glm::vec3 v) const
{
	for (const auto & prog : programs) {
		if (auto loc = glGetUniformLocation(prog.m_program, uniform); loc >= 0) {
			glUseProgram(prog.m_program);
			glUniform3fv(loc, 1, glm::value_ptr(v));
		}
	}
}

void
Shader::setModel(const Location & loc, Program pid) const
{
	auto & prog = programs[static_cast<std::size_t>(pid)];
	glUseProgram(prog.m_program);
	if (prog.model_uniform >= 0) {
		const auto model {glm::translate(loc.pos) * rotate_ypr(loc.rot)};
		glUniformMatrix4fv(prog.model_uniform, 1, GL_FALSE, glm::value_ptr(model));
	}
}

