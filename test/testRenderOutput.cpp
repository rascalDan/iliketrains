#include "testRenderOutput.h"
#include <gl_traits.h>

TestRenderOutput::TestRenderOutput(TextureAbsCoord outputSize) : size {outputSize}
{
	const auto configureAttachment = [this](glFramebuffer & fbo, glTexture<GL_TEXTURE_2D> & data, const GLenum iformat,
											 const GLenum attachment) {
		data.storage(1, iformat, size);
		data.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		data.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		fbo.texture(attachment, data);
	};
	configureAttachment(output, outImage, GL_RGBA8, GL_COLOR_ATTACHMENT0);
	output.drawBuffers(GL_COLOR_ATTACHMENT0);

	depth.storage(GL_DEPTH_COMPONENT, size);
	output.buffer(GL_DEPTH_ATTACHMENT, depth);

	output.assertComplete();
}
