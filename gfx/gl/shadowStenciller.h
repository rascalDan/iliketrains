#pragma once

#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "glFramebuffer.h"
#include "program.h"

class LightDirection;

class ShadowStenciller {
public:
	template<typename T> static constexpr T STENCIL_ANGLES = 8;

	ShadowStenciller();

	static void configureStencilTexture(glTexture<GL_TEXTURE_2D_ARRAY> &, ImageDimensions);
	void setLightDirection(const LightDirection & lightDir);
	[[nodiscard]] Direction2D getLightDirection() const;
	void renderStencil(const glTexture<GL_TEXTURE_2D_ARRAY> &, const MeshBase &, Texture::AnyPtr texture) const;

private:
	mutable glFramebuffer fbo;
	Program shadowCaster;
	Program::RequiredUniformLocation viewProjectionLoc {shadowCaster, "viewProjection"};

	Direction2D lightDir;
	std::array<glm::mat4, STENCIL_ANGLES<size_t>> viewProjections;
};
