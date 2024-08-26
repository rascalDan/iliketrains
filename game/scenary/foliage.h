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
	using LocationVertex = std::pair<glm::mat3, GlobalPosition3D>;
	mutable InstanceVertices<LocationVertex> instances;
	void render(const SceneShader &) const override;
	void shadows(const ShadowMapper &) const override;
	void updateStencil(const ShadowStenciller &) const override;
	glTexture shadowStencil = ShadowStenciller::createStencilTexture(256, 256);

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Foliage>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
