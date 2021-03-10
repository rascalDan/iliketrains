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
#include <span>

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

class RailLinks : public NetworkOf<RailLink>, public WorldObject {
public:
	RailLinks();

	std::shared_ptr<RailLink> addLinksBetween(glm::vec3 start, glm::vec3 end);

private:
	void tick(TickDuration elapsed) override;
};

#endif
