#include "sceneShader.h"
#include <gfx/gl/shaders/dynamicPoint-vert.h>
#include <gfx/gl/shaders/dynamicPointInst-vert.h>
#include <gfx/gl/shaders/fixedPoint-vert.h>
#include <gfx/gl/shaders/landmass-frag.h>
#include <gfx/gl/shaders/landmass-vert.h>
#include <gfx/gl/shaders/material-frag.h>
#include <gfx/gl/shaders/network-frag.h>
#include <gfx/gl/shaders/networkCurve-geom.h>
#include <gfx/gl/shaders/networkCurve-tesc.h>
#include <gfx/gl/shaders/networkCurve-tese.h>
#include <gfx/gl/shaders/networkCurve-vert.h>
#include <gfx/gl/shaders/networkStraight-geom.h>
#include <gfx/gl/shaders/networkStraight-vert.h>
#include <gfx/gl/shaders/pointLight-frag.h>
#include <gfx/gl/shaders/pointLight-geom.h>
#include <gfx/gl/shaders/pointLight-vert.h>
#include <gfx/gl/shaders/spotLight-frag.h>
#include <gfx/gl/shaders/spotLight-geom.h>
#include <gfx/gl/shaders/spotLight-vert.h>
#include <gfx/gl/shaders/water-frag.h>
#include <gfx/gl/shaders/water-vert.h>
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
	basicInst {dynamicPointInst_vert, material_frag}, absolute {fixedPoint_vert, material_frag},
	spotLightInst {spotLight_vert, spotLight_geom, spotLight_frag},
	pointLightInst {pointLight_vert, pointLight_geom, pointLight_frag}, landmass {landmass_vert, landmass_frag},
	networkStraight {networkStraight_vert, networkStraight_geom, network_frag},
	networkCurve {networkCurve_vert, networkCurve_tesc, networkCurve_tese, networkCurve_geom, network_frag}
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

SceneShader::BasicProgram::BasicProgram() : SceneProgram {dynamicPoint_vert, material_frag} { }

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

SceneShader::WaterProgram::WaterProgram() : SceneProgram {water_vert, water_frag} { }

void
SceneShader::WaterProgram::use(float waveCycle) const
{
	Program::use();
	glUniform(waveLoc, waveCycle);
}
