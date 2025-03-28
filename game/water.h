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

class Water : public WorldObject, public Renderable {
public:
	explicit Water(std::shared_ptr<GeoData>);

	void render(const SceneShader & shader, const Frustum &) const override;

	void tick(TickDuration) override;
	float waveCycle {0.F};

	struct Vertex {
		GlobalPosition3D pos;
	};

private:
	void generateMeshes();

	std::shared_ptr<GeoData> geoData;
	UniqueCollection<MeshT<Vertex>> meshes;
	Texture::Ptr water;
};
