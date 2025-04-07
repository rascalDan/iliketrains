#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.h"
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
	bodyMesh->configureVAO(instanceVAO)
			.addAttribs<LocationVertex, &LocationVertex::body, &LocationVertex::bodyPos>(instances.bufferName(), 1);
	bogies.front()
			->configureVAO(instancesBogiesVAO.front())
			.addAttribs<LocationVertex, &LocationVertex::front, &LocationVertex::frontPos>(instances.bufferName(), 1);
	bogies.back()
			->configureVAO(instancesBogiesVAO.back())
			.addAttribs<LocationVertex, &LocationVertex::back, &LocationVertex::backPos>(instances.bufferName(), 1);
	static_assert(sizeof(LocationVertex) == 144UL);
}

void
RailVehicleClass::render(const SceneShader & shader, const Frustum &) const
{
	if (const auto count = static_cast<GLsizei>(instances.size())) {
		if (texture) {
			texture->bind();
		}
		shader.basicInst.use();
		bodyMesh->DrawInstanced(instanceVAO, count);
		bogies.front()->DrawInstanced(instancesBogiesVAO.front(), count);
		bogies.back()->DrawInstanced(instancesBogiesVAO.back(), count);
	}
}

void
RailVehicleClass::shadows(const ShadowMapper & mapper, const Frustum &) const
{
	if (const auto count = static_cast<GLsizei>(instances.size())) {
		mapper.dynamicPointInst.use();
		bodyMesh->DrawInstanced(instanceVAO, count);
		bogies.front()->DrawInstanced(instancesBogiesVAO.front(), count);
		bogies.back()->DrawInstanced(instancesBogiesVAO.back(), count);
	}
}
