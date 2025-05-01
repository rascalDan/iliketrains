#include "testRenderOutput.h"
#include <gl_traits.h>
#include <stdexcept>

TestRenderOutput::TestRenderOutput(TextureAbsCoord outputSize) : size {outputSize}
{
	glBindFramebuffer(GL_FRAMEBUFFER, output);
	const auto configuregdata
			= [this](const GLuint data, const GLint format, const GLenum type, const GLenum attachment) {
				  glBindTexture(GL_TEXTURE_2D, data);
				  glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGBA, type, nullptr);
				  glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				  glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
			  };
	configuregdata(outImage, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
}
