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
		glBindVertexArray(instanceVAO);
		glDrawElementsInstanced(
				bodyMesh->type(), bodyMesh->count(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(count));
		glBindVertexArray(instancesBogiesVAO.front());
		glDrawElementsInstanced(bogies.front()->type(), bogies.front()->count(), GL_UNSIGNED_INT, nullptr,
				static_cast<GLsizei>(instancesBogies.front().count()));
		glBindVertexArray(instancesBogiesVAO.back());
		glDrawElementsInstanced(bogies.back()->type(), bogies.back()->count(), GL_UNSIGNED_INT, nullptr,
				static_cast<GLsizei>(instancesBogies.back().count()));
		glBindVertexArray(0);
	}
}

void
RailVehicleClass::shadows(const ShadowMapper & mapper) const
{
	if (const auto count = instancesBody.count()) {
		mapper.dynamicPointInst.use();
		glBindVertexArray(instanceVAO);
		glDrawElementsInstanced(
				bodyMesh->type(), bodyMesh->count(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(count));
		glBindVertexArray(instancesBogiesVAO.front());
		glDrawElementsInstanced(bogies.front()->type(), bogies.front()->count(), GL_UNSIGNED_INT, nullptr,
				static_cast<GLsizei>(instancesBogies.front().count()));
		glBindVertexArray(instancesBogiesVAO.back());
		glDrawElementsInstanced(bogies.back()->type(), bogies.back()->count(), GL_UNSIGNED_INT, nullptr,
				static_cast<GLsizei>(instancesBogies.back().count()));
		glBindVertexArray(0);
	}
}
