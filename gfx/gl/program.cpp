#include "program.h"
#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>

void
Program::linkAndValidate() const
{
	glLinkProgram(m_program);
	Shader::CheckShaderError(m_program, GL_LINK_STATUS, true, "Error linking shader program");

	glValidateProgram(m_program);
	Shader::CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Invalid shader program");
}

void
Program::use() const
{
	glUseProgram(m_program);
}

Program::UniformLocation::UniformLocation(GLuint program, const char * name) :
	location {glGetUniformLocation(program, name)}
{
}

Program::RequiredUniformLocation::RequiredUniformLocation(GLuint program, const char * name) :
	UniformLocation {program, name}
{
	if (location < 0) {
		throw std::logic_error(std::string {"Required uniform does not exist: "} + name);
	}
}
