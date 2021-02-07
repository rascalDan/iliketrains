#include "link.h"
#include <compare>
#include <tuple>

Link::Link(End a, End b, float l) : ends {{std::move(a), std::move(b)}}, length {l} { }

bool
operator<(const glm::vec3 & a, const glm::vec3 & b)
{
	// NOLINTNEXTLINE(hicpp-use-nullptr,modernize-use-nullptr)
	return std::tie(a.x, a.z, a.y) < std::tie(b.x, b.z, b.y);
}

bool
operator<(const Node & a, const Node & b)
{
	return a.pos < b.pos;
}
