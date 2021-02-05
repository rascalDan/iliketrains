#ifndef RAILLINKS_H
#define RAILLINKS_H

#include "collection.hpp"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/models/vertex.hpp"
#include "gfx/renderable.h"
#include "link.h"
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <vector>
class Shader;
class Texture;

// A piece of rail track
class RailLink : public Link, public Renderable {
public:
	using Link::Link;

	void render(const Shader &) const override;

protected:
	RailLink();
	Collection<Mesh, false> meshes;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class RailLinkStraight : public RailLink {
public:
	RailLinkStraight(End, End);
};

template<typename T> concept RailLinkConcept = std::is_base_of_v<RailLink, T>;

class RailLinks : public Renderable, public WorldObject {
public:
	RailLinks();
	template<RailLinkConcept T> std::shared_ptr<T> addLink(glm::vec3, glm::vec3);

private:
	using Nodes = std::set<NodePtr, PtrSorter<NodePtr>>;
	Collection<RailLink> links;
	Nodes nodes;
	void render(const Shader &) const override;
	void tick(TickDuration elapsed) override;
	std::shared_ptr<Texture> texture;
};

#endif
