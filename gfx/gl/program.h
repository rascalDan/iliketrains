#pragma once

#include "shader.h"
#include <GL/glew.h>
#include <glRef.hpp>
#include <glm/mat4x4.hpp>

class Location;

using ProgramRef = glRef<GLuint, &glCreateProgram, &glDeleteProgram>;
class Program {
public:
	template<typename... S> Program(const S &... srcs)
	{
		(glAttachShader(m_program, srcs), ...);
		linkAndValidate();
	}
	virtual ~Program() = default;

	class UniformLocation {
	public:
		UniformLocation(GLuint prog, const char * name);
		operator auto() const
		{
			return location;
		}

	protected:
		GLint location;
	};

	class RequiredUniformLocation : public UniformLocation {
	public:
		RequiredUniformLocation(GLuint prog, const char * name);
	};

	operator GLuint() const
	{
		return m_program;
	}

protected:
	void use() const;
	void linkAndValidate() const;
	ProgramRef m_program;
};
