#include "network.h"
#include "routeWalker.h"
#include <array>
#include <cache.h>
#include <filesystem>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <glm/gtx/intersect.hpp>
#include <initializer_list>
#include <ray.hpp>
#include <stdexcept>
#include <utility>

Network::Network(const std::string & tn) : texture {Texture::cachedTexture.get(tn)} { }

Node::Ptr
Network::nodeAt(glm::vec3 pos)
{
	return newNodeAt(pos).first;
}

std::pair<Node::Ptr, bool>
Network::newNodeAt(glm::vec3 pos)
{
	const auto [n, i] = candidateNodeAt(pos);
	if (!i) {
		nodes.insert(n);
	}
	return {n, !i};
}

Node::Ptr
Network::findNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return *n;
	}
	return {};
}

std::pair<Node::Ptr, bool>
Network::candidateNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return {*n, true};
	}
	return {std::make_shared<Node>(pos), false};
}

Node::Ptr
Network::intersectRayNodes(const Ray & ray) const
{
	// Click within 2m of a node
	if (const auto node = std::find_if(nodes.begin(), nodes.end(),
				[&ray](const Node::Ptr & node) {
					glm::vec3 ipos, inorm;
					return glm::intersectRaySphere(ray.start, ray.direction, node->pos, 2.F, ipos, inorm);
				});
			node != nodes.end()) {
		return *node;
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
Network::routeFromTo(const Link::End & end, const Node::Ptr & dest) const
{
	return RouteWalker().findRouteTo(end, dest);
}
