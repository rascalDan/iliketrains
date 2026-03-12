#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include <array>
#include <glm/glm.hpp>
#include <lib/resource.h>
#include <location.h>
#include <maths.h>
#include <memory>
#include <ranges>

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
			.body = {.rotation = position.getRotationTransform(), .position = position.pos},
			.front = {.rotation = position.getRotationTransform(),
					.position = {sincos(position.rot.x) * wheelBase * 0.5F, position.pos.z}},
			.back = {.rotation = position.getRotationTransform(),
					.position = {sincos(position.rot.x) * wheelBase * -0.5F, position.pos.z}},
	}));
}

void
RailVehicleClass::postLoad()
{
	texture = getTexture();
	glDebugScope _ {0};
	bodyMesh->configureVAO(instanceVAO, 0)
			.addAttribs<LocationVertex, &LocationVertex::Part::rotation, &LocationVertex::Part::position>(1);
	static_assert(sizeof(LocationVertex) == 144UL);
}

void
RailVehicleClass::renderAllParts(const size_t count) const
{
	using PartPair = std::pair<Mesh::Ptr, LocationVertex::Part LocationVertex::*>;
	const auto bufferName = instances.bufferName();
	for (const auto & [mesh, part] : {
				 PartPair {bodyMesh, &LocationVertex::body},
				 PartPair {bogies.front(), &LocationVertex::front},
				 PartPair {bogies.back(), &LocationVertex::back},
		 }) {
		instanceVAO.useBuffer<LocationVertex>(1, bufferName, part);
		mesh->drawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}

void
RailVehicleClass::render(const SceneShader & shader, const Frustum &) const
{
	if (const auto count = (instances.size())) {
		glDebugScope _ {instanceVAO};
		if (texture) {
			texture->bind(0);
		}
		shader.basicInst.use();
		renderAllParts(count);
	}
}

void
RailVehicleClass::shadows(const ShadowMapper & mapper, const Frustum &) const
{
	if (const auto count = instances.size()) {
		glDebugScope _ {instanceVAO};
		mapper.dynamicPointInst.use();
		renderAllParts(count);
	}
}
