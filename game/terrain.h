#ifndef TERRAIN_H
#define TERRAIN_H

#include "worldobject.h"
#include <GL/glew.h>
#include <array>
#include <gfx/models/vertex.hpp>
#include <gfx/renderable.h>
#include <memory>
#include <special_members.hpp>
#include <vector>

class Shader;
class Texture;

class Terrain : public WorldObject, public Renderable {
public:
	Terrain();
	~Terrain() override;
	NO_COPY(Terrain);
	NO_MOVE(Terrain);

	void render(const Shader & shader) const override;

	void tick(TickDuration) override { }

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	GLuint m_vertexArrayObject;
	std::array<GLuint, NUM_BUFFERS> m_vertexArrayBuffers;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::shared_ptr<Texture> texture;
};

#endif
