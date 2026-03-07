#include "billboardPainter.h"
#include "gl_traits.h"
#include "gldebug.h"
#include "maths.h"
#include <gfx/gl/shaders/billboardPainter-frag.h>
#include <gfx/gl/shaders/billboardPainter-geom.h>
#include <gfx/gl/shaders/billboardPainter-vert.h>
#include <stdexcept>

const auto VIEWS = []<GLint... Ep>(std::integer_sequence<GLint, Ep...>) {
	constexpr float STEP = two_pi / BillboardPainter::VIEW_ANGLES<decltype(two_pi)>;
	return std::array {rotate_yaw<4>(Ep * STEP)...};
}(std::make_integer_sequence<GLint, BillboardPainter::VIEW_ANGLES<GLint>>());

BillboardPainter::BillboardPainter() :
	program {billboardPainter_vert, billboardPainter_geom, billboardPainter_frag}, angle {}, view {}
{
	glDebugScope _ {fbo};
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	static constexpr std::array<GLenum, 2> ATTACHMENTS {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(ATTACHMENTS.size(), ATTACHMENTS.data());
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(program);
	glUniform(viewLoc, std::span<const glm::mat4> {VIEWS});
}

void
BillboardPainter::setView(const Angle newAngle, const glm::mat4 & newView)
{
	angle = newAngle;
	view = newView;
}

Angle
BillboardPainter::getAngle() const
{
	return angle;
}

void
BillboardPainter::configureBillBoardTextures(glTextures<3> & textures, GLsizei width, GLsizei height)
{
	glDebugScope _ {0};
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	const auto configuregdata = [width, height](const auto & texture, const GLint iformat, const GLenum format) {
		texture.bind(GL_TEXTURE_2D_ARRAY);
		glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, iformat, width, height, VIEW_ANGLES<GLint>, 0, format, GL_BYTE, nullptr);
	};
	configuregdata(textures[0], GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
	configuregdata(textures[1], GL_RGB8_SNORM, GL_RGB);
	configuregdata(textures[2], GL_RGB5_A1, GL_RGBA);
}

void
BillboardPainter::renderBillBoard(
		const glTextures<3> & billboard, const MeshBase & mesh, const Texture::AnyPtr texture) const
{
	glDebugScope _ {fbo};
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0, 0, 0, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, billboard[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, billboard[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, billboard[2], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Billboard framebuffer not complete!");
	}
	if (texture) {
		texture->bind();
	}
	glUseProgram(program);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const TextureDimensions billboardSize = billboard[0].getSize();
	glViewport(0, 0, billboardSize.x, billboardSize.y);
	const auto & centre = mesh.getDimensions().centre;
	const auto & size = mesh.getDimensions().size;
	glUniform(viewProjectionLoc,
			std::span<const glm::mat4> {VIEWS *
					[extentsMat = glm::translate(glm::ortho(-size, size, -size, size, -size, size), -centre), this](
							const auto & view) {
						return this->view * view * extentsMat;
					}});
	mesh.draw();
}
