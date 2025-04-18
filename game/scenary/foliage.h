#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/gl/shadowStenciller.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"

class SceneShader;
class ShadowMapper;
class Location;

class Foliage : public Asset, public Renderable, public StdTypeDefs<Foliage> {
	Mesh::Ptr bodyMesh;
	Texture::Ptr texture;
	glVertexArray instanceVAO;
	glVertexArray instancePointVAO;

public:
	[[nodiscard]] std::any createAt(const Location &) const override;

	struct LocationVertex {
		glm::mat3 rotation;
		float yaw;
		GlobalPosition3D position;
	};

	mutable InstanceVertices<LocationVertex> instances;
	void render(const SceneShader &, const Frustum &) const override;
	void shadows(const ShadowMapper &, const Frustum &) const override;
	void updateStencil(const ShadowStenciller &) const override;
	glTexture shadowStencil = ShadowStenciller::createStencilTexture(256, 256);

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Foliage>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
