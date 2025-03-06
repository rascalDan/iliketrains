#include "foliage.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.h"

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
			.addAttribs<LocationVertex, &LocationVertex::rotation, &LocationVertex::position>(
					instances.bufferName(), 1);
	VertexArrayObject {instancePointVAO}.addAttribs<LocationVertex, &LocationVertex::position, &LocationVertex::yaw>(
			instances.bufferName());
}

void
Foliage::updateStencil(const ShadowStenciller & ss) const
{
	if (instances.size() > 0) {
		ss.renderStencil(shadowStencil, *bodyMesh, texture);
	}
}

void
Foliage::render(const SceneShader & shader, const Frustum &) const
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
		const auto dimensions = bodyMesh->getDimensions();
		mapper.stencilShadowProgram.use(dimensions.centre, dimensions.size);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, shadowStencil);
		glBindVertexArray(instancePointVAO);
		glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
		glBindVertexArray(0);
	}
}
