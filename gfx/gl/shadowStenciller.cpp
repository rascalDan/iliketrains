#include "shadowStenciller.h"
#include "gfx/lightDirection.h"
#include "gfx/models/mesh.h"
#include "gl_traits.h"
#include "gldebug.h"
#include "maths.h"
#include <gfx/gl/shaders/shadowStencil-frag.h>
#include <gfx/gl/shaders/shadowStencil-geom.h>
#include <gfx/gl/shaders/shadowStencil-vert.h>
#include <stdexcept>

ShadowStenciller::ShadowStenciller() :
	shadowCaster {shadowStencil_vert, shadowStencil_geom, shadowStencil_frag}, lightDir {}, viewProjections {}
{
	glDebugScope _ {fbo};
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
ShadowStenciller::setLightDirection(const LightDirection & lightDir)
{
	this->lightDir = lightDir.position();
	viewProjections = [&lightDir]<GLint... Ep>(std::integer_sequence<GLint, Ep...>) {
		constexpr float STEP = two_pi / STENCIL_ANGLES<decltype(two_pi)>;
		return std::array {rotate_pitch<4>(half_pi - lightDir.position().y)
				* rotate_yaw<4>((Ep * STEP) - lightDir.position().x)...};
	}(std::make_integer_sequence<GLint, STENCIL_ANGLES<GLint>>());
}

Direction2D
ShadowStenciller::getLightDirection() const
{
	return lightDir;
}

void
ShadowStenciller::configureStencilTexture(glTexture<GL_TEXTURE_2D_ARRAY> & stencil, ImageDimensions size)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	stencil.storage(1, GL_DEPTH_COMPONENT16, size || STENCIL_ANGLES<GLsizei>);
	stencil.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stencil.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stencil.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	stencil.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void
ShadowStenciller::renderStencil(
		const glTexture<GL_TEXTURE_2D_ARRAY> & stencil, const MeshBase & mesh, const Texture::AnyPtr texture) const
{
	glDebugScope _ {fbo};
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, stencil, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Stencil framebuffer not complete!");
	}
	if (texture) {
		texture->bind(0);
	}
	glUseProgram(shadowCaster);
	glClear(GL_DEPTH_BUFFER_BIT);
	const auto stencilSize = stencil.getSize();
	glViewport(0, 0, stencilSize.x, stencilSize.y);
	const auto & centre = mesh.getDimensions().centre;
	const auto & size = mesh.getDimensions().size;
	glUniform(viewProjectionLoc,
			std::span<const glm::mat4> {viewProjections *
					[extentsMat = glm::translate(glm::ortho(-size, size, -size, size, -size, size), -centre)](
							const auto & viewProjection) {
						return viewProjection * extentsMat;
					}});
	mesh.draw();
}
