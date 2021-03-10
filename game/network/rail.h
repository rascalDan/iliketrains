#ifndef RAILLINKS_H
#define RAILLINKS_H

#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/renderable.h"
#include "link.h"
#include "network.h"
#include <glm/glm.hpp>
#include <location.hpp>
#include <maths.h>
#include <memory>
#include <set>
#include <span>
#include <utility>

class Shader;
class Vertex;

// A piece of rail track
class RailLink : public Link, public Renderable {
public:
	using Link::Link;

	void render(const Shader &) const override;

protected:
	[[nodiscard]] static MeshPtr defaultMesh(const std::span<Vertex> vertices);

	MeshPtr mesh;
};

class RailLinkStraight : public RailLink {
public:
	RailLinkStraight(const NodePtr &, const NodePtr &);
	[[nodiscard]] Location positionAt(float dist, unsigned char start) const override;

private:
	RailLinkStraight(NodePtr, NodePtr, const glm::vec3 & diff);
};

class RailLinkCurve : public RailLink {
public:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec2);
	[[nodiscard]] Location positionAt(float dist, unsigned char start) const override;

private:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec3, const Arc);
	glm::vec3 centreBase;
	float radius;
	Arc arc;
};

template<typename T> concept RailLinkConcept = std::is_base_of_v<RailLink, T>;

class RailLinks : public NetworkOf<RailLink>, public WorldObject {
public:
	RailLinks();
	template<RailLinkConcept T, typename... Params>
	std::shared_ptr<T>
	addLink(glm::vec3 a, glm::vec3 b, Params &&... params)
	{
		const auto node1 = *nodes.insert(std::make_shared<Node>(a)).first;
		const auto node2 = *nodes.insert(std::make_shared<Node>(b)).first;
		auto l {links.create<T>(node1, node2, std::forward<Params>(params)...)};
		joinLinks(l);
		return l;
	}

	std::shared_ptr<RailLink> addLinksBetween(glm::vec3 start, glm::vec3 end);

private:
	void tick(TickDuration elapsed) override;
	void joinLinks(const LinkPtr &) const;
};

#endif
