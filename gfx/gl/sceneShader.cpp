#include "sceneShader.h"
#include <gfx/gl/shaders/fs-landmass.h>
#include <gfx/gl/shaders/fs-material.h>
#include <gfx/gl/shaders/fs-pointLight.h>
#include <gfx/gl/shaders/fs-spotLight.h>
#include <gfx/gl/shaders/fs-water.h>
#include <gfx/gl/shaders/gs-pointLight.h>
#include <gfx/gl/shaders/gs-spotLight.h>
#include <gfx/gl/shaders/vs-dynamicPoint.h>
#include <gfx/gl/shaders/vs-dynamicPointInst.h>
#include <gfx/gl/shaders/vs-fixedPoint.h>
#include <gfx/gl/shaders/vs-pointLight.h>
#include <gfx/gl/shaders/vs-spotLight.h>
#include <gfx/gl/shaders/vs-water.h>
#include <gfx/gl/vertexArrayObject.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>

SceneShader::SceneShader() :
	basicInst {dynamicPointInst_vs, material_fs}, landmass {fixedPoint_vs, landmass_fs},
	absolute {fixedPoint_vs, material_fs}, spotLightInst {spotLight_vs, spotLight_gs, spotLight_fs},
	pointLightInst {pointLight_vs, pointLight_gs, pointLight_fs}
{
}

void
SceneShader::setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const
{
	for (const auto & prog : std::initializer_list<const SceneProgram *> {
				 &basic, &basicInst, &water, &landmass, &absolute, &pointLightInst, &spotLightInst}) {
		prog->setViewProjection(viewPoint, viewProjection);
	}
}

void
SceneShader::setViewPort(const ViewPort & viewPort) const
{
	for (const auto & prog : std::initializer_list<const SceneProgram *> {
				 &basic, &basicInst, &water, &landmass, &absolute, &pointLightInst, &spotLightInst}) {
		prog->setViewPort(viewPort);
	}
}

void
SceneShader::SceneProgram::setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const
{
	glUseProgram(*this);
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
	glUniform3iv(viewPointLoc, 1, glm::value_ptr(viewPoint));
}

void
SceneShader::SceneProgram::setViewPort(const ViewPort & viewPort) const
{
	if (viewPortLoc >= 0) {
		glUseProgram(*this);
		glUniform4iv(viewPortLoc, 1, glm::value_ptr(viewPort));
	}
}

SceneShader::BasicProgram::BasicProgram() :
	SceneProgram {dynamicPoint_vs, material_fs}, modelLoc {*this, "model"}, modelPosLoc {*this, "modelPos"}
{
}

void
SceneShader::BasicProgram::setModel(Location const & location) const
{
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rotate_ypr(location.rot)));
	glUniform3iv(modelPosLoc, 1, glm::value_ptr(location.pos));
}

void
SceneShader::BasicProgram::use(Location const & location) const
{
	Program::use();
	setModel(location);
}

SceneShader::WaterProgram::WaterProgram() : SceneProgram {water_vs, water_fs}, waveLoc {*this, "waves"} { }

void
SceneShader::WaterProgram::use(float waveCycle) const
{
	Program::use();
	glUniform1f(waveLoc, waveCycle);
}
