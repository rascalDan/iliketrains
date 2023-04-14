#pragma once

#include "program.h"
#include <glArrays.h>

class Location;

class SceneShader {
	class SceneProgram : public Program {
	public:
		template<typename... S>
		inline explicit SceneProgram(const S &... srcs) :
			Program {srcs...}, viewProjectionLoc {*this, "viewProjection"}, viewPortLoc {*this, "viewPort"}
		{
		}

		void setViewProjection(const glm::mat4 &) const;
		void setViewPort(const glm::ivec4 &) const;

	private:
		RequiredUniformLocation viewProjectionLoc;
		UniformLocation viewPortLoc;
	};

	class BasicProgram : public SceneProgram {
	public:
		BasicProgram();
		void setModel(const Location &) const;
		void use(const Location &) const;

	private:
		RequiredUniformLocation modelLoc;
	};

	class AbsolutePosProgram : public SceneProgram {
	public:
		using Program::use;
		using SceneProgram::SceneProgram;
	};

	class WaterProgram : public SceneProgram {
	public:
	public:
		WaterProgram();
		void use(float waveCycle) const;

	private:
		RequiredUniformLocation waveLoc;
	};

	class PointLightShader : public SceneProgram {
	public:
		PointLightShader();

		void add(const glm::vec3 & position, const glm::vec3 & colour, const float kq) const;

	private:
		UniformLocation colourLoc;
		UniformLocation kqLoc;
		glVertexArray va;
		glBuffer b;
	};

	class SpotLightShader : public SceneProgram {
	public:
		SpotLightShader();

		void add(const glm::vec3 & position, const glm::vec3 & direction, const glm::vec3 & colour, const float kq,
				const float arc) const;

	private:
		UniformLocation colourLoc;
		UniformLocation kqLoc;
		UniformLocation arcLoc;
		glVertexArray va;
		glBuffer b;
	};

public:
	SceneShader();

	BasicProgram basic;
	SceneProgram basicInst;
	WaterProgram water;
	AbsolutePosProgram landmass, absolute;
	PointLightShader pointLight;
	SpotLightShader spotLight;

	void setViewProjection(const glm::mat4 & viewProjection) const;
	void setViewPort(const glm::ivec4 & viewPort) const;
};
