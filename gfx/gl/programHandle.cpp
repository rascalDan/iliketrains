#include "programHandle.h"
#include "glSource.h"

ProgramHandleBase::ProgramHandleBase(GLuint vs, GLuint fs) : viewProjection_uniform {}, model_uniform {}
{
	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);

	glLinkProgram(m_program);
	GLsource::CheckShaderError(m_program, GL_LINK_STATUS, true, "Error linking shader program");

	glValidateProgram(m_program);
	GLsource::CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Invalid shader program");
}
