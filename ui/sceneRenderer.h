#pragma once

#include "gfx/gl/program.h"
#include "lib/glArrays.h"
#include <functional>
#include <glm/fwd.hpp>

class SceneRenderer {
public:
	explicit SceneRenderer(glm::ivec2 size, GLuint output);

	void render(std::function<void()> content) const;

private:
	GLuint output;
	glFrameBuffer gBuffer;
	glTexture gPosition, gNormal, gAlbedoSpec;
	glRenderBuffer depth;
	class DeferredLightProgram : public Program {
	public:
		using Program::Program;
		using Program::use;
	};
	DeferredLightProgram lighting;
	glVertexArray displayVAO;
	glBuffer displayVBO;
};
