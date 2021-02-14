#ifndef LINK_H
#define LINK_H

#include <array>
#include <gfx/gl/transform.h>
#include <glm/glm.hpp>
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
using LinkWPtr = std::weak_ptr<const Link>;
class Link {
public:
	using End = std::pair<NodePtr, float /*dir*/>;

	Link(End, End, float);
	virtual ~Link() = default;
	NO_COPY(Link);
	NO_MOVE(Link);

	[[nodiscard]] virtual Transform positionAt(float dist, unsigned char start) const = 0;

	std::array<End, 2> ends;
	float length;
	using Next = std::pair<LinkWPtr, unsigned char /*end*/>;
	std::array<std::vector<Next>, 2> nexts;
};

bool operator<(const glm::vec3 & a, const glm::vec3 & b);
bool operator<(const Node & a, const Node & b);

#endif
