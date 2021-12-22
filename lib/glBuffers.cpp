#include "glBuffers.h"

void
glBuffersBase::gen(GLsizei n, GLuint * ids)
{
	glGenBuffers(n, ids);
}

void
glBuffersBase::del(GLsizei n, const GLuint * ids)
{
	glDeleteBuffers(n, ids);
}
