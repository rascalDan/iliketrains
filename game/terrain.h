#ifndef TERRAIN_H
#define TERRAIN_H

#include "chronology.hpp"
#include "collection.hpp"
#include "game/worldobject.h"
#include <gfx/models/mesh.h>
#include <gfx/renderable.h>
#include <memory>
#include <string>
#include <vector>

class Shader;
class Texture;
class Vertex;

class Terrain : public WorldObject, public Renderable {
public:
	Terrain();
	explicit Terrain(const std::string &);

	void render(const Shader & shader) const override;

	void tick(TickDuration) override;
	float waveCycle {0.F};

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	void finish(unsigned int width, unsigned int height, std::vector<Vertex> &);

	Collection<Mesh, false> meshes;
	std::shared_ptr<Texture> grass, water;
};

#endif
