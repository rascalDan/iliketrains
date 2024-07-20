#include "shadowStenciller.h"
#include "config/types.h"
#include "gfx/gl/program.h"
#include "gfx/gl/shaders/vs-shadowStencil.h"
#include "gfx/models/mesh.h"
#include "glArrays.h"
#include "gl_traits.h"
#include "maths.h"
#include <stdexcept>

ShadowStenciller::ShadowStenciller() : shadowCaster {shadowStencil_vs}
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glTexture
ShadowStenciller::createStencilTexture(GLsizei width, GLsizei height)
{
	glTexture stencil;
	glBindTexture(GL_TEXTURE_2D, stencil);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	return stencil;
}

void
ShadowStenciller::renderStencil(const glTexture & stencil, const MeshBase & mesh, const RelativePosition3D & mins,
		const RelativePosition3D & maxs) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, stencil, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Stencil framebuffer not complete!");
	}
	glUseProgram(shadowCaster);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 256, 256);
	glEnable(GL_DEPTH_TEST);
	const auto extents = glm::ortho(mins.x, maxs.x, mins.z, maxs.z, mins.y, maxs.y);
	const auto lightDir = glm::lookAt({}, north, up);
	glUniform(viewProjectionLoc, extents * lightDir);
	mesh.Draw();
}
