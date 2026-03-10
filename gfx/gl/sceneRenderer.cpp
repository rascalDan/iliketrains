#include "sceneRenderer.h"
#include "maths.h"
#include <gfx/gl/shaders/directionalLight-frag.h>
#include <gfx/gl/shaders/lighting-frag.h>
#include <gfx/gl/shaders/lighting-vert.h>
#include <gfx/renderable.h>
#include <glm/gtc/type_ptr.hpp>

static constexpr const std::array<const glm::i8vec4, 4> displayVAOdata {{
		// positions(x,y) texture coords(z,w)
		{-1, 1, 0, 1},
		{-1, -1, 0, 0},
		{1, 1, 1, 1},
		{1, -1, 1, 0},
}};

SceneRenderer::SceneRenderer(ScreenAbsCoord s, GLuint o) : SceneRenderer {s, o, glDebugScope {o}} { }

SceneRenderer::SceneRenderer(ScreenAbsCoord s, GLuint o, glDebugScope) :
	camera {{-1250000, -1250000, 35.0F}, quarter_pi, ratio(s), 100, 10000000}, size {s}, output {o},
	lighting {lighting_vert, lighting_frag}, shadowMapper {{2048, 2048}}
{
	shader.setViewPort({0, 0, size.x, size.y});
	displayVAO.configure().addAttribs<glm::i8vec4>(0, displayVBO, displayVAOdata);

	const auto configuregdata = [this](glTexture<GL_TEXTURE_2D> & data, const GLenum iformat, const GLenum attachment) {
		data.storage(1, iformat, size);
		data.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		data.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
	};

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	configuregdata(gPosition, GL_RGB32F, GL_COLOR_ATTACHMENT0);
	configuregdata(gNormal, GL_RGB8_SNORM, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, GL_RGB8, GL_COLOR_ATTACHMENT2);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer could not be completed! (setup gBuffer)");
	}
	constexpr std::array<GLenum, 3> attachments {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(attachments.size(), attachments.data());

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	configuregdata(gIllumination, GL_RGB8, GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer could not be completed! (setup gBufferIll)");
	}
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, output);
}

void
SceneRenderer::resize(ScreenAbsCoord newSize)
{
	glDebugScope _ {output};
	size = newSize;
	camera.setAspect(ratio(size));
	const auto configuregdata = [this](glTexture<GL_TEXTURE_2D> & data, const GLenum iformat, const GLenum attachment) {
		data = {};
		data.storage(1, iformat, size);
		data.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		data.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
	};
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	configuregdata(gPosition, GL_RGB32F, GL_COLOR_ATTACHMENT0);
	configuregdata(gNormal, GL_RGB8_SNORM, GL_COLOR_ATTACHMENT1);
	configuregdata(gAlbedoSpec, GL_RGB8, GL_COLOR_ATTACHMENT2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer could not be completed! (resize gBuffer)");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	configuregdata(gIllumination, GL_RGB8, GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer could not be completed! (resize gBufferIll)");
	}
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	shader.setViewPort({0, 0, size.x, size.y});
}

void
SceneRenderer::preFrame(const SceneProvider & scene, const LightDirection lightDirection)
{
	glDebugScope _ {output};
	const auto lightView = shadowMapper.preFrame(lightDirection, camera);
	billboardPainter.setView(std::asin(camera.getForward().z), camera.getView());
	scene.forEachRenderable([&lightView, this](Renderable * renderable) {
		renderable->preFrame(camera, lightView);
		renderable->updateBillboard(billboardPainter);
	});
}

void
SceneRenderer::render(const SceneProvider & scene) const
{
	glDebugScope _ {output};
	shader.setViewProjection(camera.getPosition(), camera.getViewProjection());
	glViewport(0, 0, size.x, size.y);

	if (glDebugScope _ {gBuffer, "Geometry/colour pass"}) {
		// Geometry/colour pass - writes albedo, normal and position textures
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_PROGRAM_POINT_SIZE);
		scene.content(shader, camera);
	}

	if (glDebugScope _ {gBufferIll, "Environment pass"}) {
		// Environment pass -
		// * ambient - clears illumination texture - see setAmbientLight
		// * directional - updates shadowMapper, reads normal and position, writes illumination - see
		// setDirectionalLight
		scene.environment(shader, *this);
	}

	if (glDebugScope _ {gBufferIll, "Scene lighting pass"}) {
		// Scene lights pass -
		// * per light - reads normal and position, writes illumination
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
		glBlendFunc(GL_ONE, GL_ONE);
		gPosition.bind(0);
		gNormal.bind(1);
		shadowMapper.bind(2);
		glDisable(GL_DEPTH_TEST);
		scene.lights(shader);
	}

	if (glDebugScope _ {output, "Composition pass"}) {
		// Composition pass - reads albedo and illumination, writes output
		glBindFramebuffer(GL_FRAMEBUFFER, output);
		glViewport(0, 0, size.x, size.y);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		gAlbedoSpec.bind(2);
		gIllumination.bind(3);
		lighting.use();
		renderQuad();
	}
}

void
SceneRenderer::setAmbientLight(const RGB & colour) const
{
	glDebugScope _ {output};
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
	glClearColor(colour.r, colour.g, colour.b, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
}

void
SceneRenderer::setDirectionalLight(
		const RGB & colour, const LightDirection & direction, const SceneProvider & scene) const
{
	if (colour.r > 0 || colour.g > 0 || colour.b > 0) {
		glDebugScope _ {output};
		const auto lvp = shadowMapper.update(scene, direction, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferIll);
		glBlendFunc(GL_ONE, GL_ONE);
		gPosition.bind(0);
		gNormal.bind(1);
		shadowMapper.bind(2);
		glViewport(0, 0, size.x, size.y);
		dirLight.use();
		dirLight.setDirectionalLight(colour, direction.vector(), lvp);
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

SceneRenderer::DirectionalLightProgram::DirectionalLightProgram() : Program {lighting_vert, directionalLight_frag} { }

const auto toTextureSpaceMat = glm::translate(glm::identity<glm::mat4>(), glm::vec3 {0.5F})
		* glm::scale(glm::identity<glm::mat4>(), glm::vec3 {0.5F});

void
SceneRenderer::DirectionalLightProgram::setDirectionalLight(
		const RGB & c, const Direction3D & d, const std::span<const glm::mat4x4> lvp) const
{
	const auto toTextureSpace = [](const glm::mat4 & m) {
		return toTextureSpaceMat * m;
	};
	glUniform(colourLoc, c);
	glUniform(directionLoc, d);
	glUniform(lightViewProjectionCountLoc, static_cast<GLuint>(lvp.size()));
	glUniform(lightViewProjectionLoc, std::span<const glm::mat4> {lvp * toTextureSpace});
}
