#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
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
	bodyMesh->configureVAO(instanceVAO, 0)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(1);
	if (!spotLight.empty()) {
		instancesSpotLightVAO.emplace();
		instancesSpotLightVAO->configure()
				.addAttribs<SpotLightVertex, &SpotLightVertex::position, &SpotLightVertex::direction,
						&SpotLightVertex::colour, &SpotLightVertex::kq, &SpotLightVertex::arc>(0)
				.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(1);
		std::transform(
				spotLight.begin(), spotLight.end(), std::back_inserter(spotLightInstances), [this](const auto & s) {
					return instancesSpotLight.acquire(*s);
				});
	}
	if (!pointLight.empty()) {
		instancesPointLightVAO.emplace();
		instancesPointLightVAO->configure()
				.addAttribs<PointLightVertex, &PointLightVertex::position, &PointLightVertex::colour,
						&PointLightVertex::kq>(0)
				.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(1);
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
		glVertexArrayVertexBuffer(instanceVAO, 1, instances.bufferName(), 0, sizeof(LocationVertex));
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
			glVertexArrayVertexBuffer(
					*instancesSpotLightVAO, 0, instancesSpotLight.bufferName(), 0, sizeof(SpotLightVertex));
			glVertexArrayVertexBuffer(*instancesSpotLightVAO, 1, instances.bufferName(), 0, sizeof(LocationVertex));
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(scount), static_cast<GLsizei>(count));
		}
		if (const auto pcount = instancesPointLight.size()) {
			shader.pointLightInst.use();
			glBindVertexArray(*instancesPointLightVAO);
			glVertexArrayVertexBuffer(
					*instancesPointLightVAO, 0, instancesPointLight.bufferName(), 0, sizeof(PointLightVertex));
			glVertexArrayVertexBuffer(*instancesPointLightVAO, 1, instances.bufferName(), 0, sizeof(LocationVertex));
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(pcount), static_cast<GLsizei>(count));
		}

		glBindVertexArray(0);
	}
}
