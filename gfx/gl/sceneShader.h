#pragma once

#include "program.h"

class Location;

struct SceneShader {
	class SceneProgram : public Program {
	public:
		template<typename... S>
		inline SceneProgram(const S &... srcs) : Program {srcs...}, viewProjectionLoc {*this, "viewProjection"}
		{
		}

		void setView(const glm::mat4 &) const;

	private:
		RequiredUniformLocation viewProjectionLoc;
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

	SceneShader();

	BasicProgram basic;
	WaterProgram water;
	AbsolutePosProgram landmass, absolute;

	void setView(glm::mat4 proj) const;
};
