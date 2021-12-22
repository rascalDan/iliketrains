#include "glVertexArrays.h"

void
glVertexArraysBase::gen(GLsizei n, GLuint * ids)
{
	glGenVertexArrays(n, ids);
}

void
glVertexArraysBase::del(GLsizei n, const GLuint * ids)
{
	glDeleteVertexArrays(n, ids);
}
