#pragma once

#include "link.h"
#include <collection.hpp>
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <sorting.hpp>
#include <string>
#include <utility>
#include <variant>

class Texture;
class Shader;
class Ray;

class Network {
public:
	using LinkEnd = std::pair<LinkPtr, unsigned char>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;

	[[nodiscard]] NodePtr findNodeAt(glm::vec3) const;
	[[nodiscard]] NodePtr nodeAt(glm::vec3);
	[[nodiscard]] std::pair<NodePtr, bool> newNodeAt(glm::vec3);
	[[nodiscard]] virtual LinkPtr intersectRayLinks(const Ray &) const = 0;
	[[nodiscard]] virtual NodePtr intersectRayNodes(const Ray &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, glm::vec3) const;
	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, const NodePtr &) const;

	virtual LinkPtr addStraight(glm::vec3, glm::vec3) = 0;

protected:
	static void joinLinks(const LinkPtr & l, const LinkPtr & ol);

	using Nodes = std::set<NodePtr, PtrSorter<NodePtr>>;
	Nodes nodes;
	std::shared_ptr<Texture> texture;
};

template<typename T> class NetworkOf : public Network, public Renderable {
protected:
	using Network::Network;

	Collection<T> links;
	void joinLinks(const LinkPtr &) const;

protected:
	[[nodiscard]] LinkPtr intersectRayLinks(const Ray &) const override;

public:
	template<typename L, typename... Params>
	std::shared_ptr<L>
	addLink(glm::vec3 a, glm::vec3 b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = nodeAt(a), node2 = nodeAt(b);
		auto l {links.template create<L>(node1, node2, std::forward<Params>(params)...)};
		joinLinks(l);
		return l;
	}

	LinkPtr addStraight(glm::vec3 n1, glm::vec3 n2) override;

	void render(const Shader &) const override;
};
