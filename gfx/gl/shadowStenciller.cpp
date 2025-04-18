#include "shadowStenciller.h"
#include "gfx/gl/program.h"
#include "gfx/gl/shaders/fs-shadowStencil.h"
#include "gfx/gl/shaders/gs-shadowStencil.h"
#include "gfx/gl/shaders/vs-shadowStencil.h"
#include "gfx/lightDirection.h"
#include "gfx/models/mesh.h"
#include "glArrays.h"
#include "gl_traits.h"
#include "maths.h"
#include <stdexcept>

ShadowStenciller::ShadowStenciller() :
	shadowCaster {shadowStencil_vs, shadowStencil_gs, shadowStencil_fs}, viewProjections {}
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
ShadowStenciller::setLightDirection(const LightDirection & lightDir)
{
	viewProjections = [&lightDir]<GLint... Ep>(std::integer_sequence<GLint, Ep...>) {
		constexpr float STEP = two_pi / STENCIL_ANGLES<decltype(two_pi)>;
		return std::array {rotate_pitch<4>(half_pi - lightDir.position().y)
				* rotate_yaw<4>((Ep * STEP) - lightDir.position().x)...};
	}(std::make_integer_sequence<GLint, STENCIL_ANGLES<GLint>>());
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

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, height, STENCIL_ANGLES<GLint>, 0,
			GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

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
	const auto stencilSize = Texture::getSize(stencil);
	glViewport(0, 0, stencilSize.x, stencilSize.y);
	const auto & centre = mesh.getDimensions().centre;
	const auto & size = mesh.getDimensions().size;
	glUniform(viewProjectionLoc,
			std::span<const glm::mat4> {viewProjections *
					[extentsMat = glm::translate(glm::ortho(-size, size, -size, size, -size, size), -centre)](
							const auto & viewProjection) {
						return viewProjection * extentsMat;
					}});
	mesh.Draw();
}
