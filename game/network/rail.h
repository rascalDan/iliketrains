#pragma once

#include "chronology.hpp"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/renderable.h"
#include "link.h"
#include "network.h"
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <special_members.hpp>

class Shader;
class Vertex;
struct Arc;

// A piece of rail track
class RailLinkStraight;
class RailLink : public virtual Link, public Renderable {
public:
	RailLink() = default;
	inline ~RailLink() override = 0;

	using StraightLink = RailLinkStraight;

	void render(const Shader &) const override;
	NO_COPY(RailLink);
	NO_MOVE(RailLink);

protected:
	[[nodiscard]] glm::vec3 vehiclePositionOffset() const override;
	[[nodiscard]] static MeshPtr defaultMesh(const std::span<Vertex> vertices);

	MeshPtr mesh;
};
RailLink::~RailLink() = default;

class RailLinkStraight : public RailLink, public LinkStraight {
public:
	RailLinkStraight(const NodePtr &, const NodePtr &);

private:
	RailLinkStraight(NodePtr, NodePtr, const glm::vec3 & diff);
};

class RailLinkCurve : public RailLink, public LinkCurve {
public:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec2);

private:
	RailLinkCurve(const NodePtr &, const NodePtr &, glm::vec3, const Arc);
};

class RailLinks : public NetworkOf<RailLink>, public WorldObject {
public:
	RailLinks();

	std::shared_ptr<RailLink> addLinksBetween(glm::vec3 start, glm::vec3 end);

private:
	void tick(TickDuration elapsed) override;
};
