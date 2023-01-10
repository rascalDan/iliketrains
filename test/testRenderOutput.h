#pragma once

#include "glArrays.h"
#include <glm/vec2.hpp>
#include <special_members.hpp>

class TestRenderOutput {
public:
	TestRenderOutput(glm::ivec2 size = {640, 480});
	virtual ~TestRenderOutput() = default;

	NO_MOVE(TestRenderOutput);
	NO_COPY(TestRenderOutput);

	const glm::ivec2 size;
	glFrameBuffer output;
	glRenderBuffer depth;
	glTexture outImage;
};
template<glm::ivec2 Size> class TestRenderOutputSize : public TestRenderOutput {
public:
	TestRenderOutputSize() : TestRenderOutput {Size} { }
};
