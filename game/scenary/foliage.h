#pragma once

#include "assetFactory/asset.h"

class SceneShader;
class ShadowMapper;
class Location;
class Texture;

class Foliage : public Asset, public StdTypeDefs<Foliage> {
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;

public:
	void render(const SceneShader &, const Location &) const;
	void shadows(const ShadowMapper &, const Location &) const;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Foliage>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
