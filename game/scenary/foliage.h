#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/renderable.h"

class SceneShader;
class ShadowMapper;
class Location;
class Texture;

class Foliage : public Asset, public Renderable, public StdTypeDefs<Foliage> {
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;
	glVertexArray instanceVAO;

public:
	using LocationVertex = std::pair<glm::mat3, GlobalPosition3D>;
	mutable InstanceVertices<LocationVertex> instances;
	void render(const SceneShader &) const override;
	void shadows(const ShadowMapper &) const override;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Foliage>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
