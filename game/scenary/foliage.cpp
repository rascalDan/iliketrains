#include "foliage.h"
#include "gfx/frustum.h"
#include "gfx/gl/billboardPainter.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/shadowStenciller.h"
#include "gfx/gl/vertexArrayObject.h"
#include <location.h>

static_assert(std::is_constructible_v<Foliage>);
constexpr float OBJECT_BILLBOARD_DIVISOR = 64;
constexpr float ASSUMED_VIEWPORT = 1440;
constexpr float OVER_SAMPLE_MULTIPLIER = 4; // Use mesh until billboard 1/4 of rendered size

namespace {
	GLsizei
	billboardTextureSizeForObject(RelativeDistance objectSize)
	{
		return static_cast<GLsizei>(std::pow(2, std::ceil(std::log2(objectSize / OBJECT_BILLBOARD_DIVISOR))));
	}
}

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

	const auto & size = bodyMesh->getDimensions().size;
	billboardSize = billboardTextureSizeForObject(size);
	shadowStencil = ShadowStenciller::createStencilTexture(billboardSize, billboardSize);
	billboard = BillboardPainter::createBillBoardTextures(billboardSize, billboardSize);
	useMeshClipDist = (ASSUMED_VIEWPORT * OVER_SAMPLE_MULTIPLIER * size) / static_cast<RelativeDistance>(billboardSize);
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
		const auto & dims = bodyMesh->getDimensions();
		instancePartitions = instances.partition(
				[&frustum, &dims](const auto & location) {
					return frustum.contains(location.position + dims.centre, dims.size);
				},
				[&frustum, this](const auto & location) {
					return distance(frustum.getPosition(), location.position) < useMeshClipDist;
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
			billboard[0].bind(GL_TEXTURE_2D_ARRAY, GL_TEXTURE0);
			billboard[1].bind(GL_TEXTURE_2D_ARRAY, GL_TEXTURE1);
			billboard[2].bind(GL_TEXTURE_2D_ARRAY, GL_TEXTURE2);
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
			shadowStencil.bind(GL_TEXTURE_2D_ARRAY, GL_TEXTURE0);
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
