#pragma once

#include "gfx/lightDirection.h"
#include "glArrays.h"
#include "program.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include "shadowMapper.h"
#include <gfx/camera.h>
#include <glm/fwd.hpp>

class SceneRenderer {
public:
	explicit SceneRenderer(ScreenAbsCoord size, GLuint output);

	void resize(ScreenAbsCoord size);

	void render(const SceneProvider &) const;
	void setAmbientLight(const RGB & colour) const;
	void setDirectionalLight(const RGB & colour, const LightDirection & direction, const SceneProvider &) const;

	Camera camera;

protected:
	void renderQuad() const;

	ScreenAbsCoord size;
	GLuint output;
	glFrameBuffer gBuffer, gBufferIll;
	GLint normaliFormat;
	glTexture gPosition, gNormal, gAlbedoSpec, gIllumination;
	glRenderBuffer depth;

	class DeferredLightProgram : public Program {
	public:
		using Program::Program;
		using Program::use;
	};

	class DirectionalLightProgram : public Program {
	public:
		DirectionalLightProgram();
		using Program::use;

		void setDirectionalLight(
				const RGB &, const Direction3D &, const GlobalPosition3D &, const std::span<const glm::mat4x4>) const;

	private:
		RequiredUniformLocation directionLoc {*this, "lightDirection"};
		RequiredUniformLocation colourLoc {*this, "lightColour"};
		RequiredUniformLocation lightPointLoc {*this, "lightPoint"};
		RequiredUniformLocation lightViewProjectionLoc {*this, "lightViewProjection"};
		RequiredUniformLocation lightViewProjectionCountLoc {*this, "lightViewProjectionCount"};
	};

	DeferredLightProgram lighting;
	DirectionalLightProgram dirLight;
	glVertexArray displayVAO;
	glBuffer displayVBO;
	SceneShader shader;
	ShadowMapper shadowMapper;
};
