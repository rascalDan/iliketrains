#pragma once

#include "gfx/gl/program.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "glArrays.h"

class ShadowStenciller {
public:
	ShadowStenciller(const Direction3D & lightDir, const Direction3D & lightDirUp);

	[[nodiscard]]
	static glTexture createStencilTexture(GLsizei width, GLsizei height);
	void renderStencil(const glTexture &, const MeshBase &, const Texture::AnyPtr texture) const;

private:
	glFrameBuffer fbo;
	Program shadowCaster;
	glm::mat4 lightDirMat;
	Program::UniformLocation viewProjectionLoc {shadowCaster, "viewProjection"};
};
