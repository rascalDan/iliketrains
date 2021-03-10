#include "network.h"
#include <array>
#include <cache.h>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <initializer_list>
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
		for (const auto oe : {0, 1}) {
			for (const auto te : {0, 1}) {
				if (l->ends[te].node == ol->ends[oe].node) {
					l->ends[te].nexts.emplace_back(ol, oe);
					ol->ends[oe].nexts.emplace_back(l, te);
				}
			}
		}
	}
}
