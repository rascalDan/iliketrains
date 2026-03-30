#include "foliage.h"
#include "gfx/frustum.h"
#include "gfx/gl/billboardPainter.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/gl/shadowStenciller.h"
#include <location.h>

static_assert(std::is_constructible_v<Foliage>);
constexpr float OBJECT_BILLBOARD_DIVISOR = 64;
constexpr float BILLBOARD_ANGLE_TOLERANCE = 250.F; // Radians per mm size
constexpr float ASSUMED_VIEWPORT = 1440;
constexpr float OVER_SAMPLE_MULTIPLIER = 2; // Use mesh until billboard 1/2 of rendered size

namespace {
	GLsizei
	billboardTextureSizeForObject(RelativeDistance objectSize)
	{
		return static_cast<GLsizei>(std::pow(2, std::ceil(std::log2(objectSize / OBJECT_BILLBOARD_DIVISOR))));
	}
}

std::weak_ptr<glVertexArray> Foliage::commonInstanceVAO, Foliage::commonInstancePointVAO;

std::any
Foliage::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<InstanceVertex>::InstanceProxy>(
			instances.acquire(locationData->acquire(position)));
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
	glDebugScope _ {0};
	if (!(instanceVAO = commonInstanceVAO.lock())) {
		commonInstanceVAO = instanceVAO = std::make_shared<glVertexArray>();
		bodyMesh->configureVAO(*instanceVAO, 0).addAttribs<InstanceVertex, &InstanceVertex::location>(1);
	}
	if (!(instancePointVAO = commonInstancePointVAO.lock())) {
		commonInstancePointVAO = instancePointVAO = std::make_shared<glVertexArray>();
		instancePointVAO->configure().addAttribs<InstanceVertex, &InstanceVertex::location>(0);
	}
	const auto & size = bodyMesh->getDimensions().size;
	billboardSize = billboardTextureSizeForObject(size);
	ShadowStenciller::configureStencilTexture(shadowStencil, {billboardSize, billboardSize});
	BillboardPainter::configureBillBoardTextures(billboard, {billboardSize, billboardSize});
	useMeshClipDist = (ASSUMED_VIEWPORT * OVER_SAMPLE_MULTIPLIER * size) / static_cast<RelativeDistance>(billboardSize);
}

void
Foliage::updateStencil(const ShadowStenciller & shadowStenciller) const
{
	if (instancePartitions.second.second != instancePartitions.second.first
			&& glm::distance(shadowStenciller.getLightDirection(), shadowStencilDir)
					> BILLBOARD_ANGLE_TOLERANCE / bodyMesh->getDimensions().size) {
		shadowStenciller.renderStencil(shadowStencil, *bodyMesh, texture);
	}
}

void
Foliage::updateBillboard(const BillboardPainter & bbp) const
{
	if (instancePartitions.first != instancePartitions.second.first
			&& std::abs(bbp.getAngle() - billboardAngle) > BILLBOARD_ANGLE_TOLERANCE / bodyMesh->getDimensions().size) {
		bbp.renderBillBoard(billboard, *bodyMesh, texture);
		billboardAngle = bbp.getAngle();
	}
}

void
Foliage::preFrame(const Frustum & frustum, const Frustum & lighting)
{
	if (instances.size() > 0) {
		const auto & dims = bodyMesh->getDimensions();
		instancePartitions = instances.partition(
				[&frustum, &dims](const auto & instance) {
					return frustum.contains(instance.location->position.xyz() + dims.centre, dims.size);
				},
				[&frustum, this](const auto & instance) {
					return distance(frustum.getPosition(), instance.location->position.xyz()) < useMeshClipDist;
				},
				[&lighting, &dims](const auto & instance) {
					return lighting.contains(instance.location->position.xyz() + dims.centre, dims.size);
				});
		//         In view frustum       /            Outside view frustum            /
		// Close to view / Far from view / Casts shadow into view / No shadow in view /
	}
}

void
Foliage::render(const SceneShader & shader, const Frustum &) const
{
	if (instancePartitions.first) {
		glDebugScope _ {*instanceVAO};
		std::ignore = instances.size();
		if (const auto count = instancePartitions.first - instancePartitions.second.first) {
			glDebugScope _ {0, "Billboard"};
			const auto dimensions = bodyMesh->getDimensions();
			shader.billboard.use(dimensions.size, dimensions.centre);
			billboard[0].bind(0);
			billboard[1].bind(1);
			billboard[2].bind(2);
			glBindVertexArray(*instancePointVAO);
			instancePointVAO->useBuffer(0, instances);
			glDrawArrays(GL_POINTS, static_cast<GLint>(instancePartitions.second.first), static_cast<GLsizei>(count));
			glBindVertexArray(0);
		}
		if (const auto count = instancePartitions.second.first) {
			glDebugScope _ {0, "Mesh"};
			shader.basicInst.use();
			if (texture) {
				texture->bind(0);
			}
			instanceVAO->useBuffer(1, instances);
			bodyMesh->drawInstanced(*instanceVAO, static_cast<GLsizei>(count));
		}
	}
}

void
Foliage::shadows(const ShadowMapper & mapper, const Frustum &) const
{
	if (instancePartitions.second.second) {
		glDebugScope _ {*instanceVAO};
		std::ignore = instances.size();
		if (const auto count = instancePartitions.second.second - instancePartitions.second.first) {
			glDebugScope _ {0, "Billboard"};
			const auto dimensions = bodyMesh->getDimensions();
			mapper.stencilShadowProgram.use(dimensions.centre, dimensions.size);
			shadowStencil.bind(0);
			glBindVertexArray(*instancePointVAO);
			instancePointVAO->useBuffer(0, instances);
			glDrawArrays(GL_POINTS, static_cast<GLint>(instancePartitions.second.first), static_cast<GLsizei>(count));
			glBindVertexArray(0);
		}
		if (const auto count = instancePartitions.second.first) {
			glDebugScope _ {0, "Mesh"};
			if (texture) {
				texture->bind(3);
				mapper.dynamicPointInstWithTextures.use();
			}
			else {
				mapper.dynamicPointInst.use();
			}
			instanceVAO->useBuffer(1, instances);
			bodyMesh->drawInstanced(*instanceVAO, static_cast<GLsizei>(count));
		}
	}
}
