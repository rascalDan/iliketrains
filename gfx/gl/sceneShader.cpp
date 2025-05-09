#include "sceneShader.h"
#include <gfx/gl/shaders/fs-landmass.h>
#include <gfx/gl/shaders/fs-material.h>
#include <gfx/gl/shaders/fs-network.h>
#include <gfx/gl/shaders/fs-pointLight.h>
#include <gfx/gl/shaders/fs-spotLight.h>
#include <gfx/gl/shaders/fs-water.h>
#include <gfx/gl/shaders/gs-networkCurve.h>
#include <gfx/gl/shaders/gs-networkStraight.h>
#include <gfx/gl/shaders/gs-pointLight.h>
#include <gfx/gl/shaders/gs-spotLight.h>
#include <gfx/gl/shaders/vs-dynamicPoint.h>
#include <gfx/gl/shaders/vs-dynamicPointInst.h>
#include <gfx/gl/shaders/vs-fixedPoint.h>
#include <gfx/gl/shaders/vs-landmass.h>
#include <gfx/gl/shaders/vs-networkCurve.h>
#include <gfx/gl/shaders/vs-networkStraight.h>
#include <gfx/gl/shaders/vs-pointLight.h>
#include <gfx/gl/shaders/vs-spotLight.h>
#include <gfx/gl/shaders/vs-water.h>
#include <gfx/gl/vertexArrayObject.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>

inline void
SceneShader::allPrograms(auto member, auto &&... ps) const
{
	for (const auto & prog : std::initializer_list<const SceneProgram *> {&basic, &basicInst, &water, &landmass,
				 &absolute, &pointLightInst, &spotLightInst, &networkStraight, &networkCurve}) {
		(prog->*member)(std::forward<decltype(ps)>(ps)...);
	}
}

SceneShader::SceneShader() :
	basicInst {dynamicPointInst_vs, material_fs}, absolute {fixedPoint_vs, material_fs},
	spotLightInst {spotLight_vs, spotLight_gs, spotLight_fs},
	pointLightInst {pointLight_vs, pointLight_gs, pointLight_fs}, landmass {landmass_vs, landmass_fs},
	networkStraight {networkStraight_vs, networkStraight_gs, network_fs},
	networkCurve {networkCurve_vs, networkCurve_gs, network_fs}
{
}

void
SceneShader::setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const
{
	allPrograms(&SceneProgram::setViewProjection, viewPoint, viewProjection);
}

void
SceneShader::setViewPort(const ViewPort & viewPort) const
{
	allPrograms(&SceneProgram::setViewPort, viewPort);
}

void
SceneShader::SceneProgram::setViewProjection(const GlobalPosition3D & viewPoint, const glm::mat4 & viewProjection) const
{
	glUseProgram(*this);
	glUniform(viewProjectionLoc, viewProjection);
	glUniform(viewPointLoc, viewPoint);
}

void
SceneShader::SceneProgram::setViewPort(const ViewPort & viewPort) const
{
	if (viewPortLoc) {
		glUseProgram(*this);
		glUniform(viewPortLoc, viewPort);
	}
}

SceneShader::BasicProgram::BasicProgram() : SceneProgram {dynamicPoint_vs, material_fs} { }

void
SceneShader::BasicProgram::setModel(Location const & location) const
{
	glUniform(modelLoc, location.getRotationTransform());
	glUniform(modelPosLoc, location.pos);
}

void
SceneShader::BasicProgram::use(Location const & location) const
{
	Program::use();
	setModel(location);
}

void
SceneShader::LandmassProgram::use(const glm::vec3 colourBias) const
{
	Program::use();
	glUniform(colourBiasLos, colourBias);
}

void
SceneShader::NetworkProgram::use(
		const std::span<const glm::vec3> profile, const std::span<const float> texturePos) const
{
	Program::use();
	glUniform(profileLoc, profile);
	glUniform(texturePosLoc, texturePos);
	glUniform(profileLengthLoc, static_cast<GLuint>(profile.size()));
}

SceneShader::WaterProgram::WaterProgram() : SceneProgram {water_vs, water_fs} { }

void
SceneShader::WaterProgram::use(float waveCycle) const
{
	Program::use();
	glUniform(waveLoc, waveCycle);
}
