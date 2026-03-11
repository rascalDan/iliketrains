#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"

class SceneShader;
class ShadowMapper;
class Location;

class Foliage : public Asset, public Renderable, public StdTypeDefs<Foliage> {
	Mesh::Ptr bodyMesh;
	Texture::Ptr texture;
	std::shared_ptr<glVertexArray> instanceVAO, instancePointVAO;
	static std::weak_ptr<glVertexArray> commonInstanceVAO, commonInstancePointVAO;

public:
	[[nodiscard]] std::any createAt(const Location &) const override;

	struct LocationVertex {
		glm::mat3 rotation;
		float yaw;
		GlobalPosition3D position;
	};

	mutable InstanceVertices<LocationVertex> instances;
	void preFrame(const Frustum &, const Frustum &) override;
	void render(const SceneShader &, const Frustum &) const override;
	void shadows(const ShadowMapper &, const Frustum &) const override;
	void updateStencil(const ShadowStenciller &) const override;
	void updateBillboard(const BillboardPainter &) const override;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Foliage>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
	GLsizei billboardSize {};
	RelativeDistance useMeshClipDist {};
	mutable Direction2D shadowStencilDir {std::numeric_limits<Direction2D::value_type>::infinity()};
	glTexture<GL_TEXTURE_2D_ARRAY> shadowStencil;
	mutable Angle billboardAngle = std::numeric_limits<Angle>::infinity();
	glTextures<GL_TEXTURE_2D_ARRAY, 3> billboard;

private:
	InstanceVertices<Foliage::LocationVertex>::PartitionResult instancePartitions;
};
