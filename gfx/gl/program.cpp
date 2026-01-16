#include "program.h"
#include "gldebug.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>

void
Program::linkAndValidate() const
{
	glDebugScope _ {m_program};
	glLinkProgram(m_program);
	checkProgramError(m_program, GL_LINK_STATUS, "Error linking shader program");

	glValidateProgram(m_program);
	checkProgramError(m_program, GL_VALIDATE_STATUS, "Invalid shader program");
}

void
Program::use() const
{
	glUseProgram(m_program);
}

void
Program::checkProgramError(GLuint program, GLuint flag, std::string_view errorMessage) const
{
	GLint success = 0;

	glGetProgramiv(program, flag, &success);

	if (success == GL_FALSE) {
		std::array<GLchar, 1024> error {};
		glGetProgramInfoLog(program, error.size(), nullptr, error.data());

		throw std::runtime_error {std::format("{}: '{}'", errorMessage, error.data())};
	}
}

Program::UniformLocation::UniformLocation(GLuint program, const char * name) :
	location {glGetUniformLocation(program, name)}
{
}

Program::RequiredUniformLocation::RequiredUniformLocation(GLuint program, const char * name) :
	UniformLocation {program, name}
{
	if (location < 0) {
		throw std::logic_error {std::format("Required uniform does not exist: {}", name)};
	}
}
