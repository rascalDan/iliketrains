#pragma once

#include "config/types.h"
#include "program.h"
#include <glArrays.h>
#include <span>

class Location;

class SceneShader {
	class SceneProgram : public Program {
	public:
		using Program::Program;

		void setViewProjection(const GlobalPosition3D &, const glm::mat4 &) const;
		void setViewPort(const ViewPort &) const;

	private:
		RequiredUniformLocation viewProjectionLoc {*this, "viewProjection"};
		RequiredUniformLocation viewPointLoc {*this, "viewPoint"};
		UniformLocation viewPortLoc {*this, "viewPort"};
	};

	class BasicProgram : public SceneProgram {
	public:
		BasicProgram();
		void setModel(const Location &) const;
		void use(const Location &) const;

	private:
		RequiredUniformLocation modelLoc {*this, "model"};
		RequiredUniformLocation modelPosLoc {*this, "modelPos"};
	};

	class AbsolutePosProgram : public SceneProgram {
	public:
		using Program::use;
		using SceneProgram::SceneProgram;
	};

	class NetworkProgram : public AbsolutePosProgram {
	public:
		using AbsolutePosProgram::AbsolutePosProgram;

		void use(const std::span<const glm::vec3>, const std::span<const float>) const;

	private:
		RequiredUniformLocation profileLoc {*this, "profile"};
		RequiredUniformLocation texturePosLoc {*this, "texturePos"};
		RequiredUniformLocation profileLengthLoc {*this, "profileLength"};
	};

	class WaterProgram : public SceneProgram {
	public:
		WaterProgram();
		void use(float waveCycle) const;

	private:
		RequiredUniformLocation waveLoc {*this, "waves"};
	};

public:
	SceneShader();

	BasicProgram basic;
	WaterProgram water;
	AbsolutePosProgram basicInst, landmass, absolute, spotLightInst, pointLightInst;
	NetworkProgram networkStraight, networkCurve;

	void setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const;
	void setViewPort(const ViewPort & viewPort) const;

private:
	inline void allPrograms(auto member, auto &&... ps) const;
};
