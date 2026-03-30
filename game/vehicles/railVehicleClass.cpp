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
	return std::make_shared<InstanceVertices<InstanceVertex>::InstanceProxy>(instances.acquire(InstanceVertex {
			.body = locationData->acquire(position),
			.front = locationData->acquire(position + ((sincos(position.rot.x) * wheelBase * 0.5F) || 0.F)),
			.back = locationData->acquire(position + ((sincos(position.rot.x) * wheelBase * -0.5F) || 0.F)),
	}));
}

void
RailVehicleClass::postLoad()
{
	texture = getTexture();
	glDebugScope _ {0};
	bodyMesh->configureVAO(instanceVAO, 0).addAttribs<InstanceVertex, &InstanceVertex::body>(1);
}

void
RailVehicleClass::renderAllParts(const size_t count) const
{
	using PartPair = std::pair<Mesh::Ptr, CommonLocationInstance InstanceVertex::*>;
	const auto bufferName = instances.bufferName();
	for (const auto & [mesh, part] : {
				 PartPair {bodyMesh, &InstanceVertex::body},
				 PartPair {bogies.front(), &InstanceVertex::front},
				 PartPair {bogies.back(), &InstanceVertex::back},
		 }) {
		instanceVAO.useBuffer<InstanceVertex>(1, bufferName, part);
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
