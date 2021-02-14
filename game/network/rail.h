#ifndef RAILLINKS_H
#define RAILLINKS_H

#include "collection.hpp"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/models/vertex.hpp"
#include "gfx/renderable.h"
#include "link.h"
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>
#include <set>
#include <sorting.hpp>
#include <utility>
#include <vector>

class Shader;
class Texture;

// A piece of rail track
class RailLink : public Link, public Renderable {
public:
	using Link::Link;

	void render(const Shader &) const override;

protected:
	void defaultMesh();

	Collection<Mesh, false> meshes;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class RailLinkStraight : public RailLink {
public:
	RailLinkStraight(const NodePtr &, const NodePtr &);

private:
	RailLinkStraight(NodePtr, NodePtr, const glm::vec3 & diff);
};

class RailLinkCurve : public RailLink {
public:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec2);

private:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec3, const Arc);
	glm::vec3 centreBase;
};

template<typename T> concept RailLinkConcept = std::is_base_of_v<RailLink, T>;

class RailLinks : public Renderable, public WorldObject {
public:
	RailLinks();
	template<RailLinkConcept T, typename... Params>
	std::shared_ptr<T>
	addLink(glm::vec3 a, glm::vec3 b, Params &&... params)
	{
		const auto node1 = *nodes.insert(std::make_shared<Node>(a)).first;
		const auto node2 = *nodes.insert(std::make_shared<Node>(b)).first;
		auto l = links.create<T>(node1, node2, std::forward<Params>(params)...);
		joinLinks(l);
		return l;
	}

private:
	using Nodes = std::set<NodePtr, PtrSorter<NodePtr>>;
	Collection<RailLink> links;
	Nodes nodes;
	void render(const Shader &) const override;
	void tick(TickDuration elapsed) override;
	void joinLinks(LinkPtr) const;
	std::shared_ptr<Texture> texture;
};

#endif
