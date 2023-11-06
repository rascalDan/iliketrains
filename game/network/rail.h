#pragma once

#include "chronology.h"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/renderable.h"
#include "link.h"
#include "network.h"
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <special_members.h>

class SceneShader;
class Vertex;
struct Arc;

// A piece of rail track
class RailLinkStraight;
class RailLinkCurve;

class RailLink : public virtual Link, public Renderable {
public:
	RailLink() = default;
	inline ~RailLink() override = 0;

	using StraightLink = RailLinkStraight;
	using CurveLink = RailLinkCurve;

	void render(const SceneShader &) const override;
	NO_COPY(RailLink);
	NO_MOVE(RailLink);

protected:
	[[nodiscard]] glm::vec3 vehiclePositionOffset() const override;
	[[nodiscard]] static Mesh::Ptr defaultMesh(const std::span<Vertex> vertices);

	Mesh::Ptr mesh;
};

RailLink::~RailLink() = default;

class RailLinkStraight : public RailLink, public LinkStraight {
public:
	RailLinkStraight(const Node::Ptr &, const Node::Ptr &);

private:
	RailLinkStraight(Node::Ptr, Node::Ptr, const glm::vec3 & diff);
};

class RailLinkCurve : public RailLink, public LinkCurve {
public:
	RailLinkCurve(const Node::Ptr &, const Node::Ptr &, glm::vec2);

private:
	RailLinkCurve(const Node::Ptr &, const Node::Ptr &, glm::vec3, const Arc);
};

class RailLinks : public NetworkOf<RailLink>, public WorldObject {
public:
	RailLinks();

	std::shared_ptr<RailLink> addLinksBetween(glm::vec3 start, glm::vec3 end);

private:
	void tick(TickDuration elapsed) override;
};
