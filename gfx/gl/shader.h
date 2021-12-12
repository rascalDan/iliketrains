#ifndef SHADER_INCLUDED_H
#define SHADER_INCLUDED_H

#include "programHandle.h"
#include <GL/glew.h>
#include <array>
#include <glRef.hpp>
#include <glm/glm.hpp>
#include <string_view>

class Location;

class Shader {
public:
	enum class Program { Basic = 0, Water = 1, LandMass = 2, StaticPos = 3 };

	Shader();

	void setView(glm::mat4 view) const;
	void setModel(const Location &, Program = Program::Basic) const;
	void setUniform(const GLchar *, glm::vec3 dir) const;

private:
	class ProgramHandle : public ProgramHandleBase {
	public:
		ProgramHandle(GLuint, GLuint);
	};

	std::array<ProgramHandle, 4> programs;
};

#endif
