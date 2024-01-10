#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/vertexArrayObject.h"
#include "gfx/models/texture.h"
#include "location.h"

bool
Illuminator::SpotLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(direction) && STORE_MEMBER(colour) && STORE_MEMBER(kq)
			&& STORE_MEMBER(arc);
}

bool
Illuminator::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct)
			&& STORE_HELPER(spotLight, Persistence::Appender<decltype(spotLight)>) && Asset::persist(store);
}

void
Illuminator::postLoad()
{
	texture = getTexture();
	bodyMesh->configureVAO(instanceVAO)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
	VertexArrayObject {instancesSpotLightVAO}
			.addAttribs<SpotLightVertex, &SpotLightVertex::position, &SpotLightVertex::direction,
					&SpotLightVertex::colour, &SpotLightVertex::kq, &SpotLightVertex::arc>(
					instancesSpotLight.bufferName(), 0)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
	std::transform(spotLight.begin(), spotLight.end(), std::back_inserter(spotLightInstances), [this](const auto & s) {
		return instancesSpotLight.acquire(*s);
	});
}

void
Illuminator::render(const SceneShader & shader) const
{
	if (const auto count = instances.size()) {
		shader.basicInst.use();
		if (texture) {
			texture->bind();
		}
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}

void
Illuminator::lights(const SceneShader & shader) const
{
	if (const auto count = instances.size()) {
		if (const auto scount = instancesSpotLight.size()) {
			shader.spotLightInst.use();
			glBindVertexArray(instancesSpotLightVAO);
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(scount), static_cast<GLsizei>(count));
		}

		glBindVertexArray(0);
	}
}
