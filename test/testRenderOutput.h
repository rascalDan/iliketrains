#pragma once

#include "config/types.h"
#include "game/gamestate.h"
#include "gfx/gl/glFramebuffer.h"
#include <glm/vec2.hpp>
#include <special_members.h>

class TestRenderOutput {
public:
	explicit TestRenderOutput(TextureAbsCoord size = {640, 480});
	virtual ~TestRenderOutput() = default;

	NO_MOVE(TestRenderOutput);
	NO_COPY(TestRenderOutput);

	const TextureAbsCoord size;
	glFramebuffer output;
	glRenderbuffer depth;
	glTexture<GL_TEXTURE_2D> outImage;
	GameState gameState;
};

template<TextureAbsCoord Size> class TestRenderOutputSize : public TestRenderOutput {
public:
	TestRenderOutputSize() : TestRenderOutput {Size} { }
};
