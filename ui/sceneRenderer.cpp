#include "sceneRenderer.h"
#include "maths.h"
#include <gfx/gl/shaders/fs-directionalLight.h>
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
SceneRenderer::SceneRenderer(glm::ivec2 s, GLuint o) :
	camera {{-1250.0F, -1250.0F, 35.0F}, quarter_pi, ratio(s), 0.1F, 10000.0F}, size {s}, output {o},
	lighting {lightingShader_vs, lightingShader_fs}
{
	shader.setViewPort({0, 0, size.x, size.y});
	glBindVertexArray(displayVAO);
	glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(displayVAOdata), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(displayVAOdata), glm::value_ptr(displayVAOdata.front()));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	const auto configuregdata
			= [this](const GLuint data, const GLint format, const GLenum type, const GLenum attachment) {
				  glBindTexture(GL_TEXTURE_2D, data);
				  glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGBA, type, NULL);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
			  };
	configuregdata(gPosition, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0);
	configuregdata(gNormal, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
	configuregdata(gIllumination, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT3);

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, output);
}

void
SceneRenderer::render(const SceneProvider & scene) const
{
	shader.setViewProjection(camera.GetViewProjection());
	glViewport(0, 0, size.x, size.y);

	// Geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	static constexpr std::array<unsigned int, 3> attachments {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(attachments.size(), attachments.data());
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.content(shader);

	// Illumination pass
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	scene.environment(shader, *this);
	scene.lights(shader);

	// Lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, output);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gIllumination);
	lighting.use();
	glBindVertexArray(displayVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void
SceneRenderer::setAmbientLight(const glm::vec3 & colour) const
{
	glClearColor(colour.r, colour.g, colour.b, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
}

void
SceneRenderer::setDirectionalLight(const glm::vec3 & colour, const glm::vec3 & direction) const
{
	if (colour.r > 0 || colour.g > 0 || colour.b > 0) {
		dirLight.use();
		dirLight.setDirectionalLight(colour, direction);
		glBindVertexArray(displayVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
}

SceneRenderer::DirectionalLightProgram::DirectionalLightProgram() :
	Program {lightingShader_vs, directionalLight_fs}, directionLoc {*this, "lightDirection"}, colourLoc {*this,
																									  "lightColour"}
{
}

void
SceneRenderer::DirectionalLightProgram::setDirectionalLight(const glm::vec3 & c, const glm::vec3 & d) const
{
	glUniform3fv(colourLoc, 1, glm::value_ptr(c));
	const auto nd = glm::normalize(d);
	glUniform3fv(directionLoc, 1, glm::value_ptr(nd));
}

void
SceneRenderer::SceneProvider::environment(const SceneShader &, const SceneRenderer & renderer) const
{
	renderer.setAmbientLight({0.5F, 0.5F, 0.5F});
	renderer.setDirectionalLight({0.6F, 0.6F, 0.6F}, {1, 0, -1});
}
