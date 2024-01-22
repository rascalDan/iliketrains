#pragma once

#include "config/types.h"
#include "program.h"
#include <glArrays.h>
#include <span>

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

	class NetworkProgram : public AbsolutePosProgram {
	public:
		template<typename... S> explicit NetworkProgram(S &&...);

		void use(const std::span<const glm::vec3>, const std::span<const float>) const;

	private:
		RequiredUniformLocation profileLoc, texturePosLoc, profileLengthLoc;
	};

	class WaterProgram : public SceneProgram {
	public:
		WaterProgram();
		void use(float waveCycle) const;

	private:
		RequiredUniformLocation waveLoc;
	};

public:
	SceneShader();

	BasicProgram basic;
	WaterProgram water;
	AbsolutePosProgram basicInst, landmass, absolute, spotLightInst, pointLightInst;
	NetworkProgram networkStraight, networkCurve;

	void setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const;
	void setViewPort(const ViewPort & viewPort) const;
};
