#include "sceneShader.h"
#include <array>
#include <gfx/gl/shaders/fs-basicShader.h>
#include <gfx/gl/shaders/fs-landmassShader.h>
#include <gfx/gl/shaders/fs-waterShader.h>
#include <gfx/gl/shaders/vs-basicShader.h>
#include <gfx/gl/shaders/vs-landmassShader.h>
#include <gfx/gl/shaders/vs-waterShader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.hpp>
#include <maths.h>

SceneShader::SceneShader() :
	landmass {landmassShader_vs, landmassShader_fs}, absolute {landmassShader_vs, basicShader_fs}
{
}

void
SceneShader::setViewProjection(const glm::mat4 & viewProjection) const
{
	for (const auto & prog : std::array<const SceneProgram *, 4> {&basic, &water, &landmass, &absolute}) {
		prog->setViewProjection(viewProjection);
	}
}

void
SceneShader::setViewPort(const glm::ivec4 & viewPort) const
{
	for (const auto & prog : std::array<const SceneProgram *, 4> {&basic, &water, &landmass, &absolute}) {
		prog->setViewPort(viewPort);
	}
}

void
SceneShader::SceneProgram::setViewProjection(const glm::mat4 & viewProjection) const
{
	glUseProgram(*this);
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
}

void
SceneShader::SceneProgram::setViewPort(const glm::ivec4 & viewPort) const
{
	if (viewPortLoc >= 0) {
		glUseProgram(*this);
		glUniform4iv(viewPortLoc, 1, glm::value_ptr(viewPort));
	}
}

SceneShader::BasicProgram::BasicProgram() : SceneProgram {basicShader_vs, basicShader_fs}, modelLoc {*this, "model"} { }

void
SceneShader::BasicProgram::setModel(Location const & location) const
{
	const auto model {glm::translate(location.pos) * rotate_ypr(location.rot)};
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void
SceneShader::BasicProgram::use(Location const & location) const
{
	Program::use();
	setModel(location);
}

SceneShader::WaterProgram::WaterProgram() : SceneProgram {waterShader_vs, waterShader_fs}, waveLoc {*this, "waves"} { }

void
SceneShader::WaterProgram::use(float waveCycle) const
{
	Program::use();
	glm::vec3 waves {waveCycle, 0.F, 0.F};
	glUniform3fv(waveLoc, 1, glm::value_ptr(waves));
}
