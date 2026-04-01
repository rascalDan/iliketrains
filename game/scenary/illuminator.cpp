#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/models/texture.h" // IWYU pragma: keep
#include "util.h"
#include <location.h>

static_assert(std::is_constructible_v<Illuminator>);

std::weak_ptr<glVertexArray> Illuminator::commonInstanceVAO, Illuminator::commonInstancesSpotLightVAO,
		Illuminator::commonInstancesPointLightVAO;

std::any
Illuminator::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<InstanceVertex>::InstanceProxy>(
			instances.acquire(locationData->acquire(position)));
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
	glDebugScope _ {0};
	if (createIfRequired(instanceVAO, commonInstanceVAO)) {
		bodyMesh->configureVAO(*instanceVAO, 0).addAttribs<InstanceVertex, &InstanceVertex::location>(1);
	}
	if (!spotLight.empty()) {
		if (!(instancesSpotLightVAO = commonInstancesSpotLightVAO.lock())) {
			commonInstancesSpotLightVAO = instancesSpotLightVAO = std::make_shared<glVertexArray>();
			instancesSpotLightVAO->configure()
					.addAttribs<SpotLightVertex, &SpotLightVertex::position, &SpotLightVertex::direction,
							&SpotLightVertex::colour, &SpotLightVertex::kq, &SpotLightVertex::arc>(0)
					.addAttribs<InstanceVertex, &InstanceVertex::location>(1);
		}
		std::transform(
				spotLight.begin(), spotLight.end(), std::back_inserter(spotLightInstances), [this](const auto & s) {
					return instancesSpotLight.acquire(*s);
				});
	}
	if (!pointLight.empty()) {
		if (!(instancesPointLightVAO = commonInstancesPointLightVAO.lock())) {
			commonInstancesPointLightVAO = instancesPointLightVAO = std::make_shared<glVertexArray>();
			instancesPointLightVAO->configure()
					.addAttribs<PointLightVertex, &PointLightVertex::position, &PointLightVertex::colour,
							&PointLightVertex::kq>(0)
					.addAttribs<InstanceVertex, &InstanceVertex::location>(1);
		}
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
		glDebugScope _ {*instanceVAO};
		shader.basicInst.use();
		if (texture) {
			texture->bind(0);
		}
		instanceVAO->useBuffer(1, instances);
		bodyMesh->drawInstanced(*instanceVAO, static_cast<GLsizei>(count));
	}
}

void
Illuminator::lights(const SceneShader & shader) const
{
	if (const auto count = instances.size()) {
		glDebugScope _ {*instanceVAO};
		if (const auto scount = instancesSpotLight.size()) {
			glDebugScope _ {*instancesSpotLightVAO, "Spot lights"};
			shader.spotLightInst.use();
			glBindVertexArray(*instancesSpotLightVAO);
			instancesSpotLightVAO->useBuffer(0, instancesSpotLight);
			instancesSpotLightVAO->useBuffer(1, instances);
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(scount), static_cast<GLsizei>(count));
		}
		if (const auto pcount = instancesPointLight.size()) {
			glDebugScope _ {*instancesPointLightVAO, "Point llights"};
			shader.pointLightInst.use();
			glBindVertexArray(*instancesPointLightVAO);
			instancesPointLightVAO->useBuffer(0, instancesPointLight);
			instancesPointLightVAO->useBuffer(1, instances);
			glDrawArraysInstanced(GL_POINTS, 0, static_cast<GLsizei>(pcount), static_cast<GLsizei>(count));
		}

		glBindVertexArray(0);
	}
}
