#pragma once

#include "glArrays.h"
#include <glm/vec2.hpp>

class TestRenderOutput {
public:
	TestRenderOutput();
	const glm::ivec2 size;
	glFrameBuffer output;
	glRenderBuffer depth;
	glTexture outImage;
};
