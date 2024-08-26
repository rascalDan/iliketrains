#pragma once

#include "shader.h" // IWYU pragma: export
#include <glRef.h>
#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <special_members.h>

class Location;

using ProgramRef = glRef<GLuint, &glCreateProgram, &glDeleteProgram>;

class Program {
public:
	Program() = delete;

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

		explicit
		operator bool() const
		{
			return location >= 0;
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
	void checkProgramError(GLuint program, GLuint flag, std::string_view errorMessage) const;
	void use() const;
	void linkAndValidate() const;
	ProgramRef m_program;
};
