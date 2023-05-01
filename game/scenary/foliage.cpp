#include "foliage.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.h"
#include "gfx/models/texture.h"
#include "location.h"

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
	if (const auto count = instances.size()) {
		shader.basicInst.use();
		if (texture) {
			texture->bind();
		}
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}

void
Foliage::shadows(const ShadowMapper & mapper) const
{
	if (const auto count = instances.size()) {
		mapper.dynamicPointInst.use();
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}
