#pragma once

#include "gfx/gl/shader.h"
#include "lib/glArrays.h"
#include <functional>

class SceneRenderer {
public:
	SceneRenderer(glm::ivec2 size);

	void render(std::function<void()> content) const;

private:
	glFrameBuffer gBuffer;
	glTexture gPosition, gNormal, gAlbedoSpec;
	glRenderBuffer depth;
	ProgramHandleBase lighting;
	glVertexArray displayVAO;
	glBuffer displayVBO;
};
