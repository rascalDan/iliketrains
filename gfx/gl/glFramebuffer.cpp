#include "glFramebuffer.h"
#include <stdexcept>

void
Impl::glRenderbuffer::storage(const GLenum iformat, const ImageDimensions dims)
{
	glNamedRenderbufferStorage(name, iformat, dims.x, dims.y);
}

void
Impl::glFramebuffer::buffer(const GLenum attachment, const Impl::glRenderbuffer & buffer)
{
	glNamedFramebufferRenderbuffer(name, attachment, GL_RENDERBUFFER, buffer);
}

void
Impl::glFramebuffer::drawBuffers(const std::span<const GLenum> buffers)
{
	glNamedFramebufferDrawBuffers(name, static_cast<GLsizei>(buffers.size()), buffers.data());
}

void
Impl::glFramebuffer::assertComplete() const
{
	if (glCheckNamedFramebufferStatus(name, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
}
