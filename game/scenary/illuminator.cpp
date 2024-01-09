#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/vertexArrayObject.h"
#include "gfx/models/texture.h"
#include "location.h"

bool
Illuminator::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

void
Illuminator::postLoad()
{
	texture = getTexture();
	bodyMesh->configureVAO(instanceVAO)
			.addAttribs<LocationVertex, &LocationVertex::first, &LocationVertex::second>(instances.bufferName(), 1);
}

void
Illuminator::render(const SceneShader & shader) const
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
Illuminator::lights(const SceneShader &) const
{
	if (const auto count = instances.size()) {
		// shader.pointLight.use();
		// bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
	}
}
