#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.hpp"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <glm/glm.hpp>
#include <iterator>
#include <lib/resource.h>
#include <location.hpp>
#include <map>
#include <maths.h>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

bool
RailVehicleClass::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(length) && STORE_MEMBER(wheelBase) && STORE_MEMBER(maxSpeed)
			&& STORE_NAME_HELPER("bogie", bogies, Asset::MeshArrayConstruct)
			&& STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

void
RailVehicleClass::postLoad()
{
	texture = getTexture();
	bodyMesh->configureVAO(instanceVAO).addAttribs<glm::mat4>(instancesBody.bufferName(), 1);
	bogies.front()
			->configureVAO(instancesBogiesVAO.front())
			.addAttribs<glm::mat4>(instancesBogies.front().bufferName(), 1);
	bogies.back()
			->configureVAO(instancesBogiesVAO.back())
			.addAttribs<glm::mat4>(instancesBogies.back().bufferName(), 1);
}

void
RailVehicleClass::render(const SceneShader & shader) const
{
	if (const auto count = instancesBody.count()) {
		if (texture) {
			texture->bind();
		}
		shader.basicInst.use();
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
		bogies.front()->DrawInstanced(
				instancesBogiesVAO.front(), static_cast<GLsizei>(instancesBogies.front().count()));
		bogies.back()->DrawInstanced(instancesBogiesVAO.back(), static_cast<GLsizei>(instancesBogies.back().count()));
	}
}

void
RailVehicleClass::shadows(const ShadowMapper & mapper) const
{
	if (const auto count = instancesBody.count()) {
		mapper.dynamicPointInst.use();
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
		bogies.front()->DrawInstanced(
				instancesBogiesVAO.front(), static_cast<GLsizei>(instancesBogies.front().count()));
		bogies.back()->DrawInstanced(instancesBogiesVAO.back(), static_cast<GLsizei>(instancesBogies.back().count()));
	}
}
