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
	using IntersectRayResult = std::variant<std::nullptr_t, NodePtr, LinkEnd, LinkPtr>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;

	[[nodiscard]] NodePtr findNodeAt(glm::vec3) const;
	[[nodiscard]] NodePtr nodeAt(glm::vec3);
	[[nodiscard]] std::pair<NodePtr, bool> newNodeAt(glm::vec3);
	[[nodiscard]] IntersectRayResult intersectRay(const Ray &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, glm::vec3) const;
	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, const NodePtr &) const;

protected:
	static void joinLinks(const LinkPtr & l, const LinkPtr & ol);
	[[nodiscard]] virtual IntersectRayResult intersectRayLinks(const Ray &) const = 0;

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
	[[nodiscard]] IntersectRayResult intersectRayLinks(const Ray &) const override;

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

	void render(const Shader &) const override;
};
