#pragma once

#include "gfx/gl/camera.h"
#include "gfx/gl/program.h"
#include "gfx/gl/sceneShader.h"
#include "lib/glArrays.h"
#include <functional>
#include <glm/fwd.hpp>

class SceneRenderer {
public:
	class SceneProvider {
	public:
		virtual ~SceneProvider() = default;
		virtual void content(const SceneShader &) const = 0;
		virtual void environment(const SceneShader &, const SceneRenderer &) const;
	};

	explicit SceneRenderer(glm::ivec2 size, GLuint output);

	void render(const SceneProvider &) const;
	void setAmbientLight(const glm::vec3 & colour) const;
	void setDirectionalLight(const glm::vec3 & colour, const glm::vec3 & direction) const;

	Camera camera;

private:
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

		void setDirectionalLight(const glm::vec3 &, const glm::vec3 &) const;

	private:
		RequiredUniformLocation directionLoc, colourLoc;
	};

	DeferredLightProgram lighting;
	DirectionalLightProgram dirLight;
	glVertexArray displayVAO;
	glBuffer displayVBO;
	SceneShader shader;
};
