#pragma once

#include <array>
#include <glm/glm.hpp>
#include <location.hpp>
#include <maths.h>
#include <memory>
#include <special_members.hpp>
#include <utility>
#include <vector>

// Generic network node
// something that can be travelled to
// it has location
class Node {
public:
	explicit Node(glm::vec3 p) noexcept : pos(p) {};
	virtual ~Node() noexcept = default;
	NO_COPY(Node);
	NO_MOVE(Node);

	glm::vec3 pos;
};
using NodePtr = std::shared_ptr<Node>;

// Generic network link
// something that can be travelled along
// it joins 2 nodes
class Link;
using LinkPtr = std::shared_ptr<Link>;
using LinkCPtr = std::shared_ptr<const Link>;
using LinkWPtr = std::weak_ptr<const Link>;
class Link {
public:
	using Next = std::pair<LinkWPtr, unsigned char /*end*/>;
	using Nexts = std::vector<Next>;

	struct End {
		NodePtr node;
		float dir;
		Nexts nexts {};
	};

	Link(End, End, float);
	virtual ~Link() = default;
	NO_COPY(Link);
	NO_MOVE(Link);

	[[nodiscard]] virtual Location positionAt(float dist, unsigned char start) const = 0;

	std::array<End, 2> ends;
	float length;

protected:
	[[nodiscard]] virtual glm::vec3
	vehiclePositionOffset() const
	{
		return {};
	}
};

bool operator<(const glm::vec3 & a, const glm::vec3 & b);
bool operator<(const Node & a, const Node & b);

class LinkStraight : public virtual Link {
public:
	LinkStraight() = default;
	inline ~LinkStraight() override = 0;
	NO_COPY(LinkStraight);
	NO_MOVE(LinkStraight);

	[[nodiscard]] Location positionAt(float dist, unsigned char start) const override;
};
LinkStraight::~LinkStraight() = default;

class LinkCurve : public virtual Link {
public:
	inline ~LinkCurve() override = 0;
	LinkCurve(glm::vec3, float, Arc);
	NO_COPY(LinkCurve);
	NO_MOVE(LinkCurve);

	[[nodiscard]] Location positionAt(float dist, unsigned char start) const override;

	glm::vec3 centreBase;
	float radius;
	Arc arc;
};
LinkCurve::~LinkCurve() = default;
