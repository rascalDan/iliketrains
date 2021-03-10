#include "network.h"
#include <cache.h>
#include <game/network/link.h>
#include <gfx/models/texture.h>
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
