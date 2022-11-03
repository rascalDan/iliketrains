#pragma once

#include "programHandle.h"
#include <GL/glew.h>
#include <array>
#include <glm/glm.hpp>

class Location;

class SceneShader {
public:
	enum class Program { Basic = 0, Water = 1, LandMass = 2, StaticPos = 3 };

	SceneShader();

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
