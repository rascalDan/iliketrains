#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/vertexArrayObject.h"
#include "gfx/models/texture.h" // IWYU pragma: keep
#include <location.h>

static_assert(std::is_constructible_v<Illuminator>);

std::any
Illuminator::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<LocationVertex>::InstanceProxy>(
			instances.acquire(position.getRotationTransform(), position.pos));
}

bool
Illuminator::SpotLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(direction) && STORE_MEMBER(colour) && STORE_MEMBER(kq)
			&& STORE_MEMBER(arc);
}

bool
Illuminator::PointLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(colour) && STORE_MEMBER(kq);
}

bool
Illuminator::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct)
			&& STORE_HELPER(pointLight, Persistence::Appender<decltype(pointLight)>)
			&& STORE_HELPER(spotLight, Persistence::Appender<decltype(spotLight)>) && Asset::persist(store);
}

void
Illuminator::postLoad()
{
	if (spotLight.empty() && pointLight.empty()) {
		throw std::logic_error {"Illuminator has no lights"};
	}
	texture = getTexture();
	bodyMesh->configureVAO(instanceVAO)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
	if (!spotLight.empty()) {
		instancesSpotLightVAO.emplace();
		VertexArrayObject {*instancesSpotLightVAO}
				.addAttribs<SpotLightVertex, &SpotLightVertex::position, &SpotLightVertex::direction,
						&SpotLightVertex::colour, &SpotLightVertex::kq, &SpotLightVertex::arc>(
						instancesSpotLight.bufferName(), 0)
				.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
		std::transform(
				spotLight.begin(), spotLight.end(), std::back_inserter(spotLightInstances), [this](const auto & s) {
					return instancesSpotLight.acquire(*s);
				});
	}
	if (!pointLight.empty()) {
		instancesPointLightVAO.emplace();
		VertexArrayObject {*instancesPointLightVAO}
				.addAttribs<PointLightVertex, &PointLightVertex::position, &PointLightVertex::colour,
						&PointLightVertex::kq>(instancesPointLight.bufferName(), 0)
				.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
		std::transform(
				pointLight.begin(), pointLight.end(), std::back_inserter(pointLightInstances), [this](const auto & s) {
					return instancesPointLight.acquire(*s);
				});
	}
}

void
Illuminator::render(const SceneShader & shader, const Frustum &) const
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
			glBindVertexArray(*instancesSpotLightVAO);
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(scount), static_cast<GLsizei>(count));
		}
		if (const auto pcount = instancesPointLight.size()) {
			shader.pointLightInst.use();
			glBindVertexArray(*instancesPointLightVAO);
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(pcount), static_cast<GLsizei>(count));
		}

		glBindVertexArray(0);
	}
}
