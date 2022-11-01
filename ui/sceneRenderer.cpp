#include "sceneRenderer.h"
#include <gfx/gl/shaders/fs-lightingShader.h>
#include <gfx/gl/shaders/vs-lightingShader.h>

SceneRenderer::SceneRenderer(glm::ivec2 size) : lighting {lightingShader_vs.compile(), lightingShader_fs.compile()}
{
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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void
renderQuad()
{
	if (quadVAO == 0) {
		float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,
				1.0f,
				0.0f,
				0.0f,
				1.0f,
				-1.0f,
				-1.0f,
				0.0f,
				0.0f,
				0.0f,
				1.0f,
				1.0f,
				0.0f,
				1.0f,
				1.0f,
				1.0f,
				-1.0f,
				0.0f,
				1.0f,
				0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void
SceneRenderer::render(std::function<void()> content) const
{
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	// Geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	content();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting pass
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(lighting.m_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	// TODO Configure lights
	renderQuad();
}
