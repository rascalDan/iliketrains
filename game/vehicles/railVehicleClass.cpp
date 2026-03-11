#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include <array>
#include <glm/glm.hpp>
#include <lib/resource.h>
#include <location.h>
#include <maths.h>
#include <memory>

bool
RailVehicleClass::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(length) && STORE_MEMBER(wheelBase) && STORE_MEMBER(maxSpeed)
			&& STORE_NAME_HELPER("bogie", bogies, Asset::MeshArrayConstruct)
			&& STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

std::any
RailVehicleClass::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<LocationVertex>::InstanceProxy>(instances.acquire(LocationVertex {
			.body = position.getRotationTransform(),
			.front = position.getRotationTransform(),
			.back = position.getRotationTransform(),
			.bodyPos = position.pos,
			.frontPos = {sincos(position.rot.x) * wheelBase * 0.5F, position.pos.z},
			.backPos = {sincos(position.rot.x) * wheelBase * -0.5F, position.pos.z},
	}));
}

void
RailVehicleClass::postLoad()
{
	texture = getTexture();
	glDebugScope _ {0};
	bodyMesh->configureVAO(instanceVAO, 0)
			.addAttribs<LocationVertex, &LocationVertex::body, &LocationVertex::bodyPos>(1);
	bogies.front()
			->configureVAO(instancesBogiesVAO.front(), 0)
			.addAttribs<LocationVertex, &LocationVertex::front, &LocationVertex::frontPos>(1);
	bogies.back()
			->configureVAO(instancesBogiesVAO.back(), 0)
			.addAttribs<LocationVertex, &LocationVertex::back, &LocationVertex::backPos>(1);
	static_assert(sizeof(LocationVertex) == 144UL);
}

void
RailVehicleClass::render(const SceneShader & shader, const Frustum &) const
{
	if (const auto count = static_cast<GLsizei>(instances.size())) {
		glDebugScope _ {instanceVAO};
		if (texture) {
			texture->bind(0);
		}
		shader.basicInst.use();
		instanceVAO.useBuffer(1, instances);
		instancesBogiesVAO.front().useBuffer(1, instances);
		instancesBogiesVAO.back().useBuffer(1, instances);
		bodyMesh->drawInstanced(instanceVAO, count);
		bogies.front()->drawInstanced(instancesBogiesVAO.front(), count);
		bogies.back()->drawInstanced(instancesBogiesVAO.back(), count);
	}
}

void
RailVehicleClass::shadows(const ShadowMapper & mapper, const Frustum &) const
{
	if (const auto count = static_cast<GLsizei>(instances.size())) {
		glDebugScope _ {instanceVAO};
		mapper.dynamicPointInst.use();
		bodyMesh->drawInstanced(instanceVAO, count);
		bogies.front()->drawInstanced(instancesBogiesVAO.front(), count);
		bogies.back()->drawInstanced(instancesBogiesVAO.back(), count);
	}
}
