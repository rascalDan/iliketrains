#pragma once

#include "chronology.hpp"
#include "collection.hpp"
#include "game/worldobject.h"
#include <gfx/models/mesh.h>
#include <gfx/renderable.h>
#include <memory>

class Shader;
class Texture;
class GeoData;

class Terrain : public WorldObject, public Renderable {
public:
	explicit Terrain(std::shared_ptr<GeoData>);

	void render(const Shader & shader) const override;

	void tick(TickDuration) override;
	float waveCycle {0.F};

private:
	void generateMeshes();

	std::shared_ptr<GeoData> geoData;
	Collection<Mesh, false> meshes;
	std::shared_ptr<Texture> grass, water;
};
