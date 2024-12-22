#pragma once

#include "chronology.h"
#include "game/worldobject.h"
#include "gfx/renderable.h"
#include "link.h"
#include "network.h"
#include <glm/glm.hpp>
#include <memory>
#include <special_members.h>

class SceneShader;
class Vertex;
struct Arc;

// A piece of rail track
class RailLinkStraight;
class RailLinkCurve;

class RailLink : public virtual Link {
public:
	RailLink() = default;
	inline ~RailLink() override = 0;

	using StraightLink = RailLinkStraight;
	using CurveLink = RailLinkCurve;

	NO_COPY(RailLink);
	NO_MOVE(RailLink);

protected:
	[[nodiscard]] RelativePosition3D vehiclePositionOffset() const override;
};

RailLink::~RailLink() = default;

class RailLinks;

class RailLinkStraight : public RailLink, public LinkStraight {
public:
	RailLinkStraight(NetworkLinkHolder<RailLinkStraight> &, const Node::Ptr &, const Node::Ptr &);

	struct Vertex {
		GlobalPosition3D a, b;
		glm::mat2 rotation;
		float textureRepeats;
	};

private:
	RailLinkStraight(NetworkLinkHolder<RailLinkStraight> &, Node::Ptr, Node::Ptr, const RelativePosition3D & diff);
	InstanceVertices<Vertex>::InstanceProxy instance;
};

class RailLinkCurve : public RailLink, public LinkCurve {
public:
	RailLinkCurve(NetworkLinkHolder<RailLinkCurve> &, const Node::Ptr &, const Node::Ptr &, GlobalPosition2D);

	struct Vertex {
		GlobalPosition3D a, b, c;
		float textureRepeats;
		float aangle, bangle, radius;
	};

private:
	RailLinkCurve(NetworkLinkHolder<RailLinkCurve> &, const Node::Ptr &, const Node::Ptr &, GlobalPosition3D centreBase,
			RelativeDistance radius, Arc);
	InstanceVertices<Vertex>::InstanceProxy instance;
};

template<> NetworkLinkHolder<RailLinkStraight>::NetworkLinkHolder();
template<> NetworkLinkHolder<RailLinkCurve>::NetworkLinkHolder();

class RailLinks : public NetworkOf<RailLink, RailLinkStraight, RailLinkCurve>, public WorldObject {
public:
	RailLinks();

	std::shared_ptr<RailLink> addLinksBetween(GlobalPosition3D start, GlobalPosition3D end);
	void render(const SceneShader &) const override;

	[[nodiscard]] const Surface * getBaseSurface() const override;
	[[nodiscard]] RelativeDistance getBaseWidth() const override;

private:
	void tick(TickDuration elapsed) override;
};
