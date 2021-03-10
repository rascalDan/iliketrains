#ifndef NETWORK_H
#define NETWORK_H

#include "link.h"
#include <collection.hpp>
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <sorting.hpp>
#include <string>

class Texture;
class Shader;

class Network {
public:
	explicit Network(const std::string & textureName);

	[[nodiscard]] NodePtr findNodeAt(glm::vec3) const;

protected:
	using Nodes = std::set<NodePtr, PtrSorter<NodePtr>>;
	Nodes nodes;
	std::shared_ptr<Texture> texture;
};

template<typename T> class NetworkOf : public Network, public Renderable {
protected:
	using Network::Network;

	Collection<T> links;

public:
	void render(const Shader &) const override;
};

#endif
