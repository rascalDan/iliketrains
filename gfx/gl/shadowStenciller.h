#pragma once

#include "gfx/gl/program.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "glArrays.h"

class ShadowStenciller {
public:
	ShadowStenciller();

	[[nodiscard]]
	static glTexture createStencilTexture(GLsizei width, GLsizei height);
	void setLightDirection(const Direction3D & lightDir, const Direction3D & lightDirUp);
	void renderStencil(const glTexture &, const MeshBase &, const Texture::AnyPtr texture) const;

private:
	glFrameBuffer fbo;
	Program shadowCaster;
	Program::RequiredUniformLocation viewProjectionLoc {shadowCaster, "viewProjection"};

	glm::mat4 lightDirMat {};
	std::array<glm::mat4, 8> viewProjections;
};
