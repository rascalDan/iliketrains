#pragma once

#include "config/types.h"
#include "program.h"
#include <glArrays.h>

class Location;

class SceneShader {
	class SceneProgram : public Program {
	public:
		template<typename... S>
		inline explicit SceneProgram(const S &... srcs) :
			Program {srcs...}, viewProjectionLoc {*this, "viewProjection"}, viewPointLoc {*this, "viewPoint"},
			viewPortLoc {*this, "viewPort"}
		{
		}

		void setViewProjection(const GlobalPosition3D &, const glm::mat4 &) const;
		void setViewPort(const ViewPort &) const;

	private:
		RequiredUniformLocation viewProjectionLoc, viewPointLoc;
		UniformLocation viewPortLoc;
	};

	class BasicProgram : public SceneProgram {
	public:
		BasicProgram();
		void setModel(const Location &) const;
		void use(const Location &) const;

	private:
		RequiredUniformLocation modelLoc;
		RequiredUniformLocation modelPosLoc;
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

		void add(const Position3D & position, const RGB & colour, const float kq) const;

	private:
		UniformLocation colourLoc;
		UniformLocation kqLoc;
		UniformLocation viewPointLoc;
		glVertexArray va;
		glBuffer b;
	};

	class SpotLightShader : public SceneProgram {
	public:
		SpotLightShader();

		void add(const Position3D & position, const Direction3D & direction, const RGB & colour, const float kq,
				const float arc) const;

	private:
		UniformLocation directionLoc;
		UniformLocation colourLoc;
		UniformLocation kqLoc;
		UniformLocation arcLoc;
		UniformLocation viewPointLoc;
		glVertexArray va;
		glBuffer b;
	};

public:
	SceneShader();

	BasicProgram basic;
	WaterProgram water;
	AbsolutePosProgram basicInst, landmass, absolute;
	PointLightShader pointLight;
	SpotLightShader spotLight;

	void setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const;
	void setViewPort(const ViewPort & viewPort) const;
};
