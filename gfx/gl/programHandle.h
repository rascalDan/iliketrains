#ifndef PROGRAM_HANDLE_H
#define PROGRAM_HANDLE_H

#include <GL/glew.h>
#include <glRef.hpp>

class ProgramHandleBase {
public:
	ProgramHandleBase(GLuint, GLuint);
	using ProgramRef = glRef<GLuint, &__glewCreateProgram, &__glewDeleteProgram>;

	ProgramRef m_program;
	GLint viewProjection_uniform, model_uniform;
};

#endif