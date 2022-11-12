#include "sceneRenderer.h"
#include <gfx/gl/shaders/fs-lightingShader.h>
#include <gfx/gl/shaders/vs-lightingShader.h>
#include <glm/gtc/type_ptr.hpp>

static constexpr std::array<glm::vec4, 4> displayVAOdata {{
		// positions(x,y) texture coords(z,w)
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{-1.0f, -1.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, -1.0f, 1.0f, 0.0f},
}};
SceneRenderer::SceneRenderer(glm::ivec2 size) : lighting {lightingShader_vs, lightingShader_fs}
{
	glBindVertexArray(displayVAO);
	glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(displayVAOdata), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(displayVAOdata), glm::value_ptr(displayVAOdata.front()));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	const auto configuregdata
			= [size](const GLuint data, const GLint format, const GLenum type, const GLenum attachment) {
				  glBindTexture(GL_TEXTURE_2D, data);
				  glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGBA, type, NULL);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
			  };
	configuregdata(gPosition, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0);
	configuregdata(gNormal, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
	static constexpr std::array<unsigned int, 3> attachments {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(attachments.size(), attachments.data());

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
SceneRenderer::render(std::function<void()> content) const
{
	// Geometry pass
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	content();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting pass
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	// TODO Configure lights
	lighting.use();
	glBindVertexArray(displayVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
