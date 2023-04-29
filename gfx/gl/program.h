#pragma once

#include "shader.h"
#include <GL/glew.h>
#include <glRef.h>
#include <glm/mat4x4.hpp>
#include <special_members.h>

class Location;

using ProgramRef = glRef<GLuint, &glCreateProgram, &glDeleteProgram>;
class Program {
public:
	template<typename... S> explicit Program(const S &... srcs)
	{
		(glAttachShader(m_program, srcs.compile()), ...);
		linkAndValidate();
	}
	virtual ~Program() = default;
	DEFAULT_MOVE_NO_COPY(Program);

	class UniformLocation {
	public:
		UniformLocation(GLuint prog, const char * name);
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
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

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	operator GLuint() const
	{
		return m_program;
	}

protected:
	void use() const;
	void linkAndValidate() const;
	ProgramRef m_program;
};
