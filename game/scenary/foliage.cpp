#include "foliage.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.hpp"
#include "gfx/models/texture.h"
#include "location.hpp"

bool
Foliage::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

void
Foliage::postLoad()
{
	texture = getTexture();
	bodyMesh->configureVAO(instanceVAO).addAttribs<glm::mat4>(instances.bufferName(), 1);
}

void
Foliage::render(const SceneShader & shader) const
{
	if (const auto count = instances.count()) {
		shader.basicInst.use();
		if (texture) {
			texture->bind();
		}
		glBindVertexArray(instanceVAO);
		glDrawElementsInstanced(
				bodyMesh->type(), bodyMesh->count(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(count));
		glBindVertexArray(0);
	}
}

void
Foliage::shadows(const ShadowMapper & mapper) const
{
	if (const auto count = instances.count()) {
		mapper.dynamicPointInst.use();
		glBindVertexArray(instanceVAO);
		glDrawElementsInstanced(
				bodyMesh->type(), bodyMesh->count(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(count));
		glBindVertexArray(0);
	}
}
