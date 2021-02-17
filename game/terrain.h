#ifndef TERRAIN_H
#define TERRAIN_H

#include "collection.hpp"
#include "worldobject.h"
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.hpp>
#include <gfx/renderable.h>
#include <memory>
#include <string>
#include <vector>

class Shader;
class Texture;

class Terrain : public WorldObject, public Renderable {
public:
	Terrain();
	explicit Terrain(const std::string &);

	void render(const Shader & shader) const override;

	void tick(TickDuration) override { }

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	void finish(unsigned int width, unsigned int height, std::vector<Vertex> &);
	void addWater(unsigned int width, unsigned int height, unsigned int resolution);

	Collection<Mesh, false> meshes;
	std::shared_ptr<Texture> grass, water;
};

#endif
