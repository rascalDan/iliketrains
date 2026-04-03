#include "renderable.h"
#include "gfx/gl/sceneShader.h"
#include "gl_traits.h"
#include "location.h"
#include "maths.h"
#include "util.h"

std::weak_ptr<Renderable::CommonLocationData> Renderable::commonLocationData;
std::weak_ptr<Renderable::CommonSpotLights> Renderable::commonSpotLights;
std::weak_ptr<Renderable::CommonPointLights> Renderable::commonPointLights;
std::weak_ptr<glVertexArray> Renderable::commonInstancesSpotLightVAO, Renderable::commonInstancesPointLightVAO;

Renderable::CommonLocation::CommonLocation(Location const & location) :
	position {location.pos, 0}, rotation {location.rot, 0}, rotationMatrix {location.getRotationTransform()}
{
}

Renderable::CommonLocation &
Renderable::CommonLocation ::operator=(Location const & location)
{
	position = location.pos || 0;
	rotation = location.rot || 0.F;
	rotationMatrix = location.getRotationTransform();
	return *this;
}

Renderable::Renderable()
{
	createIfRequired(locationData, commonLocationData);
	createIfRequired(spotLights, commonSpotLights);
	createIfRequired(pointLights, commonPointLights);
	if (createIfRequired(instancesSpotLightVAO, commonInstancesSpotLightVAO)) {
		instancesSpotLightVAO->configure()
				.addAttribs<SpotLightVertex, &SpotLightVertex::position, &SpotLightVertex::direction,
						&SpotLightVertex::colour, &SpotLightVertex::kq, &SpotLightVertex::arc,
						&SpotLightVertex::parentObject>(0);
	}
	if (createIfRequired(instancesPointLightVAO, commonInstancesPointLightVAO)) {
		instancesPointLightVAO->configure()
				.addAttribs<PointLightVertex, &PointLightVertex::position, &PointLightVertex::colour,
						&PointLightVertex::kq, &PointLightVertex::parentObject>(0);
	}
}

GLuint
gl_traits<InstanceVertices<Renderable::CommonLocation>::InstanceProxy>::vertexArrayAttribFormat(
		GLuint vao, GLuint index, GLuint offset)
{
	return gl_traits<
			decltype(InstanceVertices<Renderable::CommonLocation>::InstanceProxy::index)>::vertexArrayAttribFormat(vao,
			index, offset + offsetof(InstanceVertices<Renderable::CommonLocation>::InstanceProxy, index));
};

void
Renderable::preFrame(const Frustum &, const Frustum &)
{
}

void
Renderable::lights(const SceneShader & shader)
{
	glDebugScope _ {0};
	if (const auto instancesSpotLight = commonSpotLights.lock()) {
		if (const auto scount = instancesSpotLight->size()) {
			if (const auto instancesSpotLightVAO = commonInstancesSpotLightVAO.lock()) {
				glDebugScope _ {*instancesSpotLightVAO, "Spot lights"};
				shader.spotLightInst.use();
				glBindVertexArray(*instancesSpotLightVAO);
				instancesSpotLightVAO->useBuffer(0, *instancesSpotLight);
				glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(scount));
			}
		}
	}
	if (const auto instancesPointLight = commonPointLights.lock()) {
		if (const auto pcount = instancesPointLight->size()) {
			if (const auto instancesPointLightVAO = commonInstancesPointLightVAO.lock()) {
				glDebugScope _ {*instancesPointLightVAO, "Point lights"};
				shader.pointLightInst.use();
				glBindVertexArray(*instancesPointLightVAO);
				instancesPointLightVAO->useBuffer(0, *instancesPointLight);
				glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(pcount));
			}
		}
	}
}

void
Renderable::shadows(const ShadowMapper &, const Frustum &) const
{
}

void
Renderable::updateStencil(const ShadowStenciller &) const
{
}

void
Renderable::updateBillboard(const BillboardPainter &) const
{
}
