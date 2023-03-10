#pragma once

#include "camera.h"
#include "glArrays.h"
#include "program.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include "shadowMapper.h"
#include <functional>
#include <glm/fwd.hpp>

class SceneRenderer {
public:
	explicit SceneRenderer(glm::ivec2 size, GLuint output);

	void render(const SceneProvider &) const;
	void setAmbientLight(const glm::vec3 & colour) const;
	void setDirectionalLight(const glm::vec3 & colour, const glm::vec3 & direction, const SceneProvider &) const;

	Camera camera;

private:
	void renderQuad() const;

	glm::ivec2 size;
	GLuint output;
	glFrameBuffer gBuffer;
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

		void setDirectionalLight(const glm::vec3 &, const glm::vec3 &, const std::span<const glm::mat4x4>,
				const std::span<const glm::vec4>, std::size_t maps) const;

	private:
		RequiredUniformLocation directionLoc, colourLoc, lightViewProjectionLoc, lightViewProjectionCountLoc,
				lightViewShadowMapRegionLoc;
	};

	DeferredLightProgram lighting;
	DirectionalLightProgram dirLight;
	glVertexArray displayVAO;
	glBuffer displayVBO;
	SceneShader shader;
	ShadowMapper shadowMapper;
};
