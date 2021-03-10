#include "network.h"
#include <cache.h>
#include <game/network/link.h>
#include <gfx/models/texture.h>

Network::Network(const std::string & tn) : texture {Texture::cachedTexture.get(tn)} { }

NodePtr
Network::findNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(std::make_shared<Node>(pos)); n != nodes.end()) {
		return *n;
	}
	return {};
}
