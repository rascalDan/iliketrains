#include "shadowStenciller.h"
#include "gfx/gl/program.h"
#include "gfx/gl/shaders/fs-shadowStencil.h"
#include "gfx/gl/shaders/vs-shadowStencil.h"
#include "gfx/models/mesh.h"
#include "glArrays.h"
#include "gl_traits.h"
#include <stdexcept>

ShadowStenciller::ShadowStenciller() : shadowCaster {shadowStencil_vs, shadowStencil_fs}
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
ShadowStenciller::setLightDirection(const Direction3D & lightDir, const Direction3D & lightDirUp)
{
	lightDirMat = glm::lookAt(-lightDir, {}, lightDirUp);
}

glTexture
ShadowStenciller::createStencilTexture(GLsizei width, GLsizei height)
{
	glTexture stencil;
	glBindTexture(GL_TEXTURE_2D_ARRAY, stencil);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, height, 8, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE,
			nullptr);

	return stencil;
}

void
ShadowStenciller::renderStencil(const glTexture & stencil, const MeshBase & mesh, const Texture::AnyPtr texture) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, stencil, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Stencil framebuffer not complete!");
	}
	if (texture) {
		texture->bind();
	}
	glUseProgram(shadowCaster);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 256, 256);
	const auto & centre = mesh.getDimensions().centre;
	const auto & size = mesh.getDimensions().size;
	const auto extentsMat
			= glm::translate(glm::ortho(-size, size, -size, size, -size, size), {-centre.x, -centre.z, -centre.y});
	glUniform(viewProjectionLoc, extentsMat * lightDirMat);
	mesh.Draw();
}
