#include "sceneShader.h"
#include <array>
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
#include <gfx/gl/vertexArrayObject.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.hpp>
#include <maths.h>

SceneShader::SceneShader() :
	basicInst {dynamicPointInst_vs, material_fs}, landmass {fixedPoint_vs, landmass_fs},
	absolute {fixedPoint_vs, material_fs}
{
}

void
SceneShader::setViewProjection(const glm::mat4 & viewProjection) const
{
	for (const auto & prog :
			std::array<const SceneProgram *, 6> {&basic, &water, &landmass, &absolute, &pointLight, &spotLight}) {
		prog->setViewProjection(viewProjection);
	}
}

void
SceneShader::setViewPort(const glm::ivec4 & viewPort) const
{
	for (const auto & prog :
			std::array<const SceneProgram *, 6> {&basic, &water, &landmass, &absolute, &pointLight, &spotLight}) {
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

SceneShader::BasicProgram::BasicProgram() : SceneProgram {dynamicPoint_vs, material_fs}, modelLoc {*this, "model"} { }

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

SceneShader::WaterProgram::WaterProgram() : SceneProgram {water_vs, water_fs}, waveLoc {*this, "waves"} { }

void
SceneShader::WaterProgram::use(float waveCycle) const
{
	Program::use();
	glm::vec3 waves {waveCycle, 0.F, 0.F};
	glUniform3fv(waveLoc, 1, glm::value_ptr(waves));
}

SceneShader::PointLightShader::PointLightShader() :
	SceneProgram {pointLight_vs, pointLight_gs, pointLight_fs}, colourLoc {*this, "colour"}, kqLoc {*this, "kq"}
{
	VertexArrayObject<glm::vec3>::configure(va, b);
}

void
SceneShader::PointLightShader::add(const glm::vec3 & position, const glm::vec3 & colour, const float kq) const
{
	Program::use();
	glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, b);
	glUniform3fv(colourLoc, 1, glm::value_ptr(colour));
	glUniform1f(kqLoc, kq);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(position));
	glDrawArrays(GL_POINTS, 0, 1);
}

SceneShader::SpotLightShader::SpotLightShader() :
	SceneProgram {spotLight_vs, spotLight_gs, spotLight_fs}, directionLoc {*this, "v_direction"},
	colourLoc {*this, "colour"}, kqLoc {*this, "kq"}, arcLoc {*this, "arc"}
{
	using v3pair = std::pair<glm::vec3, glm::vec3>;
	VertexArrayObject<v3pair>::configure<&v3pair::first, &v3pair::second>(va, b);
}

void
SceneShader::SpotLightShader::add(const glm::vec3 & position, const glm::vec3 & direction, const glm::vec3 & colour,
		const float kq, const float arc) const
{
	Program::use();
	glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, b);
	glUniform3fv(colourLoc, 1, glm::value_ptr(colour));
	glUniform3fv(directionLoc, 1, glm::value_ptr(direction));
	glUniform1f(kqLoc, kq);
	glUniform1f(arcLoc, arc);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(position));
	glDrawArrays(GL_POINTS, 0, 1);
}
