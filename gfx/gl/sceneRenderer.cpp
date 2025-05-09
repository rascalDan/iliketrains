#include "sceneRenderer.h"
#include "maths.h"
#include "vertexArrayObject.h"
#include <gfx/gl/shaders/fs-directionalLight.h>
#include <gfx/gl/shaders/fs-lighting.h>
#include <gfx/gl/shaders/vs-lighting.h>
#include <glm/gtc/type_ptr.hpp>

static constexpr const std::array<const glm::i8vec4, 4> displayVAOdata {{
		// positions(x,y) texture coords(z,w)
		{-1, 1, 0, 1},
		{-1, -1, 0, 0},
		{1, 1, 1, 1},
		{1, -1, 1, 0},
}};

SceneRenderer::SceneRenderer(ScreenAbsCoord s, GLuint o) :
	camera {{-1250000, -1250000, 35.0F}, quarter_pi, ratio(s), 100, 10000000}, size {s}, output {o},
	lighting {lighting_vs, lighting_fs}, shadowMapper {{2048, 2048}}
{
	shader.setViewPort({0, 0, size.x, size.y});
	VertexArrayObject {displayVAO}.addAttribs<glm::i8vec4>(displayVBO, displayVAOdata);

	const auto configuregdata = [this](const GLuint data, const std::initializer_list<GLint> iformats,
										const GLenum format, const GLenum attachment) {
		glBindTexture(GL_TEXTURE_2D, data);
		glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		for (const auto iformat : iformats) {
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, size.x, size.y, 0, format, GL_BYTE, nullptr);

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
				return iformat;
			}
		}
		throw std::runtime_error("Framebuffer could not be completed!");
	};

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	configuregdata(gPosition, {GL_RGB32I}, GL_RGB_INTEGER, GL_COLOR_ATTACHMENT0);
	normaliFormat = configuregdata(gNormal, {GL_RGB8_SNORM, GL_RGB16F}, GL_RGB, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, {GL_RGB8}, GL_RGB, GL_COLOR_ATTACHMENT2);
	constexpr std::array<unsigned int, 3> attachments {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(attachments.size(), attachments.data());

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	configuregdata(gIllumination, {GL_RGB8}, GL_RGB, GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, output);
}

void
SceneRenderer::resize(ScreenAbsCoord newSize)
{
	size = newSize;
	camera.setAspect(ratio(size));
	const auto configuregdata = [this](const GLuint data, const GLint iformat, const GLenum format) {
		glBindTexture(GL_TEXTURE_2D, data);
		glTexImage2D(GL_TEXTURE_2D, 0, iformat, size.x, size.y, 0, format, GL_BYTE, nullptr);
	};
	configuregdata(gPosition, GL_RGB32I, GL_RGB_INTEGER);
	configuregdata(gNormal, normaliFormat, GL_RGB);
	configuregdata(gAlbedoSpec, GL_RGB8, GL_RGB);
	configuregdata(gIllumination, GL_RGB8, GL_RGB);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	shader.setViewPort({0, 0, size.x, size.y});
}

void
SceneRenderer::render(const SceneProvider & scene) const
{
	shader.setViewProjection(camera.getPosition(), camera.getViewProjection());
	glViewport(0, 0, size.x, size.y);

	// Geometry/colour pass - writes albedo, normal and position textures
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.content(shader, camera);

	// Environment pass -
	// * ambient - clears illumination texture - see setAmbientLight
	// * directional - updates shadowMapper, reads normal and position, writes illumination - see setDirectionalLight
	scene.environment(shader, *this);

	// Scene lights pass -
	// * per light - reads normal and position, writes illumination
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	glBlendFunc(GL_ONE, GL_ONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapper);
	glDisable(GL_DEPTH_TEST);
	scene.lights(shader);

	// Composition pass - reads albedo and illumination, writes output
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
	renderQuad();
}

void
SceneRenderer::setAmbientLight(const RGB & colour) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	glClearColor(colour.r, colour.g, colour.b, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
}

void
SceneRenderer::setDirectionalLight(
		const RGB & colour, const LightDirection & direction, const SceneProvider & scene) const
{
	if (colour.r > 0 || colour.g > 0 || colour.b > 0) {
		const auto lvp = shadowMapper.update(scene, direction, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
		glBlendFunc(GL_ONE, GL_ONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapper);
		glViewport(0, 0, size.x, size.y);
		dirLight.use();
		dirLight.setDirectionalLight(colour, direction.vector(), camera.getPosition(), lvp);
		renderQuad();
	}
}

void
SceneRenderer::renderQuad() const
{
	glBindVertexArray(displayVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

SceneRenderer::DirectionalLightProgram::DirectionalLightProgram() : Program {lighting_vs, directionalLight_fs} { }

const auto toTextureSpaceMat = glm::translate(glm::identity<glm::mat4>(), glm::vec3 {0.5F})
		* glm::scale(glm::identity<glm::mat4>(), glm::vec3 {0.5F});

void
SceneRenderer::DirectionalLightProgram::setDirectionalLight(
		const RGB & c, const Direction3D & d, const GlobalPosition3D & p, const std::span<const glm::mat4x4> lvp) const
{
	const auto toTextureSpace = [](const glm::mat4 & m) {
		return toTextureSpaceMat * m;
	};
	glUniform(colourLoc, c);
	glUniform(directionLoc, d);
	glUniform(lightPointLoc, p);
	glUniform(lightViewProjectionCountLoc, static_cast<GLuint>(lvp.size()));
	glUniform(lightViewProjectionLoc, std::span<const glm::mat4> {lvp * toTextureSpace});
}
