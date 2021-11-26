#include "network.h"
#include "routeWalker.h"
#include <array>
#include <cache.h>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <initializer_list>
#include <stdexcept>
#include <utility>

Network::Network(const std::string & tn) : texture {Texture::cachedTexture.get(tn)} { }

NodePtr
Network::nodeAt(glm::vec3 pos)
{
	return *nodes.insert(std::make_shared<Node>(pos)).first;
}

std::pair<NodePtr, bool>
Network::newNodeAt(glm::vec3 pos)
{
	const auto i = nodes.insert(std::make_shared<Node>(pos));
	return {*i.first, i.second};
}

NodePtr
Network::findNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(std::make_shared<Node>(pos)); n != nodes.end()) {
		return *n;
	}
	return {};
}

void
Network::joinLinks(const LinkPtr & l, const LinkPtr & ol)
{
	if (l != ol) {
		for (const auto oe : {0U, 1U}) {
			for (const auto te : {0U, 1U}) {
				if (l->ends[te].node == ol->ends[oe].node) {
					l->ends[te].nexts.emplace_back(ol, oe);
					ol->ends[oe].nexts.emplace_back(l, te);
				}
			}
		}
	}
}

Link::Nexts
Network::routeFromTo(const Link::End & start, glm::vec3 dest) const
{
	auto destNode {findNodeAt(dest)};
	if (!destNode) {
		throw std::out_of_range("Node does not exist in network");
	}
	return routeFromTo(start, destNode);
}

Link::Nexts
Network::routeFromTo(const Link::End & end, const NodePtr & dest) const
{
	return RouteWalker().findRouteTo(end, dest);
}
