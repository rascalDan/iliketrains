#pragma once

#include "config/types.h"
#include "gfx/gl/program.h"
#include "gfx/models/mesh.h"
#include "glArrays.h"

class ShadowStenciller {
public:
	ShadowStenciller();

	[[nodiscard]]
	static glTexture createStencilTexture(GLsizei width, GLsizei height);
	void renderStencil(const glTexture &, const MeshBase &) const;

private:
	glFrameBuffer fbo;
	Program shadowCaster;
	Program::UniformLocation viewProjectionLoc {shadowCaster, "viewProjection"};
};
