#pragma once

#include <array>
#include <glm/glm.hpp>
#include <location.h>
#include <maths.h>
#include <special_members.h>
#include <stdTypeDefs.h>
#include <utility>
#include <vector>

template<typename> class Ray;

// Generic network node
// something that can be travelled to
// it has location
class Node : public StdTypeDefs<Node> {
public:
	explicit Node(GlobalPosition3D p) noexcept : pos(p) { };
	virtual ~Node() noexcept = default;
	NO_COPY(Node);
	NO_MOVE(Node);

	GlobalPosition3D pos;
};

// Generic network link
// something that can be travelled along
// it joins 2 nodes
class Link : public StdTypeDefs<Link> {
public:
	using Next = std::pair<WPtr, unsigned char /*end*/>;
	using Nexts = std::vector<Next>;

	struct End {
		Node::Ptr node;
		float dir;
		Nexts nexts {};
	};

	Link(End, End, RelativeDistance length);
	virtual ~Link() = default;
	NO_COPY(Link);
	NO_MOVE(Link);

	[[nodiscard]] virtual Location positionAt(RelativeDistance dist, unsigned char start) const = 0;
	[[nodiscard]] virtual bool intersectRay(const Ray<GlobalPosition3D> &) const = 0;

	std::array<End, 2> ends;
	float length;

protected:
	[[nodiscard]] virtual RelativePosition3D
	vehiclePositionOffset() const
	{
		return {};
	}
};

bool operator<(const GlobalPosition3D & a, const GlobalPosition3D & b);
bool operator<(const Node & a, const Node & b);

class LinkStraight : public virtual Link {
public:
	LinkStraight() = default;
	inline ~LinkStraight() override = 0;
	NO_COPY(LinkStraight);
	NO_MOVE(LinkStraight);

	[[nodiscard]] Location positionAt(RelativeDistance dist, unsigned char start) const override;
	[[nodiscard]] bool intersectRay(const Ray<GlobalPosition3D> &) const override;
};

LinkStraight::~LinkStraight() = default;

class LinkCurve : public virtual Link {
public:
	inline ~LinkCurve() override = 0;
	LinkCurve(GlobalPosition3D centreBase, RelativeDistance radius, Arc);
	NO_COPY(LinkCurve);
	NO_MOVE(LinkCurve);

	[[nodiscard]] Location positionAt(RelativeDistance dist, unsigned char start) const override;
	[[nodiscard]] bool intersectRay(const Ray<GlobalPosition3D> &) const override;

	GlobalPosition3D centreBase;
	RelativeDistance radius;
	Arc arc;
};

LinkCurve::~LinkCurve() = default;
