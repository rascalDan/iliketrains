#pragma once

#include "chronology.h"
#include "collection.h"
#include "game/worldobject.h"
#include <gfx/models/mesh.h>
#include <gfx/renderable.h>
#include <memory>

class SceneShader;
class Texture;
class GeoData;

class Water : public WorldObject, public Renderable {
public:
	explicit Water(std::shared_ptr<GeoData>);

	void render(const SceneShader & shader) const override;

	void tick(TickDuration) override;
	float waveCycle {0.F};

private:
	void generateMeshes();

	std::shared_ptr<GeoData> geoData;
	Collection<Mesh, false> meshes;
	std::shared_ptr<Texture> water;
};
