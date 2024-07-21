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
	bodyMesh->configureVAO(instanceVAO)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
	ShadowStenciller ss;
	ss.renderStencil(shadowStencil, *bodyMesh, texture);
	Texture::saveDepth(shadowStencil, std::format("/tmp/stencil-{}.tga", id).c_str());
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
		mapper.dynamicPointInstWithTextures.use();
		if (texture) {
			texture->bind(GL_TEXTURE3);
		}
		bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}
