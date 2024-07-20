#pragma once

#include "chronology.h"
#include "collection.h"
#include "config/types.h"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"
#include <memory>

class SceneShader;
class GeoData;

class Terrain : public WorldObject, public Renderable {
public:
	explicit Terrain(std::shared_ptr<GeoData>);

	void render(const SceneShader & shader) const override;
	void shadows(const ShadowMapper &) const override;

	void tick(TickDuration) override;

	struct Vertex {
		GlobalPosition3D pos;
		Normal3D normal;
		RGB colourBias;
	};

private:
	void generateMeshes();

	std::shared_ptr<GeoData> geoData;
	Collection<MeshT<Vertex>, false> meshes;
	Texture::Ptr grass;
};
