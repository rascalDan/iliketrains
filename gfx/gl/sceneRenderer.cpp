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
	lighting {lightingShader_vs, lightingShader_fs}, shadowMapper {{4096, 4096}}
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
			= [this](const GLuint data, const std::initializer_list<GLint> formats, const GLenum attachment) {
				  glBindTexture(GL_TEXTURE_2D, data);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				  for (const auto format : formats) {
					  glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGB, GL_BYTE, NULL);

					  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
					  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
						  return format;
					  }
				  }
				  throw std::runtime_error("Framebuffer could not be completed!");
			  };
	configuregdata(gPosition, {GL_RGB32F}, GL_COLOR_ATTACHMENT0);
	configuregdata(gNormal, {GL_RGB8_SNORM, GL_RGB16F}, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, {GL_RGB8}, GL_COLOR_ATTACHMENT2);
	configuregdata(gIllumination, {GL_RGB8}, GL_COLOR_ATTACHMENT3);

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	glBindFramebuffer(GL_FRAMEBUFFER, output);
}

void
SceneRenderer::render(const SceneProvider & scene) const
{
	shader.setViewProjection(camera.getViewProjection());
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
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMapper);
	scene.environment(shader, *this);
	scene.lights(shader);

	// Lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, output);
	glViewport(0, 0, size.x, size.y);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
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
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClearColor(colour.r, colour.g, colour.b, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
}

void
SceneRenderer::setDirectionalLight(
		const glm::vec3 & colour, const glm::vec3 & direction, const SceneProvider & scene) const
{
	if (colour.r > 0 || colour.g > 0 || colour.b > 0) {
		const auto lvp = shadowMapper.update(scene, direction, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glViewport(0, 0, size.x, size.y);
		dirLight.use();
		dirLight.setDirectionalLight(colour, direction, lvp);
		glBindVertexArray(displayVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}
}

SceneRenderer::DirectionalLightProgram::DirectionalLightProgram() :
	Program {lightingShader_vs, directionalLight_fs}, directionLoc {*this, "lightDirection"},
	colourLoc {*this, "lightColour"}, lightViewProjectionLoc {*this, "lightViewProjection"},
	lightViewProjectionCountLoc {*this, "lightViewProjectionCount"}, lightViewShadowMapRegionLoc {
																			 *this, "shadowMapRegion"}
{
}

void
SceneRenderer::DirectionalLightProgram::setDirectionalLight(
		const glm::vec3 & c, const glm::vec3 & d, const std::span<const glm::mat4x4> lvp) const
{
	constexpr glm::vec4 shadowMapRegions {0.5F, 0.5F, 0.5F, 0.5F};
	glUniform3fv(colourLoc, 1, glm::value_ptr(c));
	const auto nd = glm::normalize(d);
	glUniform3fv(directionLoc, 1, glm::value_ptr(nd));
	glUniform1ui(lightViewProjectionCountLoc, static_cast<GLuint>(lvp.size()));
	glUniformMatrix4fv(lightViewProjectionLoc, static_cast<GLsizei>(lvp.size()), GL_FALSE, glm::value_ptr(lvp.front()));
	glUniform4fv(lightViewShadowMapRegionLoc, 1, glm::value_ptr(shadowMapRegions));
}
