#include "foliage.h"
#include "gfx/frustum.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/vertexArrayObject.h"
#include <location.h>

static_assert(std::is_constructible_v<Foliage>);

std::any
Foliage::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<LocationVertex>::InstanceProxy>(
			instances.acquire(position.getRotationTransform(), position.rot.y, position.pos));
}

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
	if (instancePartitions.second.second != instancePartitions.second.first) {
		ss.renderStencil(shadowStencil, *bodyMesh, texture);
	}
}

void
Foliage::updateBillboard(const BillboardPainter & bbp) const
{
	if (instancePartitions.first != instancePartitions.second.first) {
		bbp.renderBillBoard(billboard, *bodyMesh, texture);
	}
}

void
Foliage::preFrame(const Frustum & frustum, const Frustum & lighting)
{
	if (instances.size() > 0) {
		const auto dims = bodyMesh->getDimensions();
		instancePartitions = instances.partition(
				[&frustum, &dims](const auto & location) {
					return frustum.contains(location.position + dims.centre, dims.size);
				},
				[&frustum](const auto & location) {
					return distance(frustum.getPosition(), location.position) < 140'000.F;
				},
				[&lighting, &dims](const auto & location) {
					return lighting.contains(location.position + dims.centre, dims.size);
				});
		//         In view frustum       /            Outside view frustum            /
		// Close to view / Far from view / Casts shadow into view / No shadow in view /
	}
}

void
Foliage::render(const SceneShader & shader, const Frustum &) const
{
	if (instancePartitions.first) {
		glDebugScope _ {instancePointVAO};
		std::ignore = instances.size();
		if (const auto count = instancePartitions.first - instancePartitions.second.first) {
			glDebugScope _ {0, "Billboard"};
			const auto dimensions = bodyMesh->getDimensions();
			shader.billboard.use(dimensions.size, dimensions.centre);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, billboard[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, billboard[1]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, billboard[2]);
			glBindVertexArray(instancePointVAO);
			glDrawArrays(GL_POINTS, static_cast<GLint>(instancePartitions.second.first), static_cast<GLsizei>(count));
			glBindVertexArray(0);
		}
		if (const auto count = instancePartitions.second.first) {
			glDebugScope _ {0, "Mesh"};
			shader.basicInst.use();
			if (texture) {
				texture->bind();
			}
			bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
		}
	}
}

void
Foliage::shadows(const ShadowMapper & mapper, const Frustum &) const
{
	if (instancePartitions.second.second) {
		glDebugScope _ {instancePointVAO};
		std::ignore = instances.size();
		if (const auto count = instancePartitions.second.second - instancePartitions.second.first) {
			const auto dimensions = bodyMesh->getDimensions();
			mapper.stencilShadowProgram.use(dimensions.centre, dimensions.size);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, shadowStencil);
			glBindVertexArray(instancePointVAO);
			glDrawArrays(GL_POINTS, static_cast<GLint>(instancePartitions.second.first), static_cast<GLsizei>(count));
			glBindVertexArray(0);
		}
		if (const auto count = instancePartitions.second.first) {
			if (texture) {
				texture->bind(GL_TEXTURE3);
				mapper.dynamicPointInstWithTextures.use();
			}
			else {
				mapper.dynamicPointInst.use();
			}
			bodyMesh->DrawInstanced(instanceVAO, static_cast<GLsizei>(count));
		}
	}
}
