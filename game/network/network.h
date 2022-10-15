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
	using LinkEnd = std::pair<Link::Ptr, unsigned char>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;

	[[nodiscard]] Node::Ptr findNodeAt(glm::vec3) const;
	[[nodiscard]] Node::Ptr nodeAt(glm::vec3);
	[[nodiscard]] std::pair<Node::Ptr, bool> newNodeAt(glm::vec3);
	[[nodiscard]] std::pair<Node::Ptr, bool> candidateNodeAt(glm::vec3) const;
	[[nodiscard]] virtual Link::Ptr intersectRayLinks(const Ray &) const = 0;
	[[nodiscard]] virtual Node::Ptr intersectRayNodes(const Ray &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, glm::vec3) const;
	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, const Node::Ptr &) const;

	virtual Link::CCollection candidateStraight(glm::vec3, glm::vec3) = 0;
	virtual Link::CCollection candidateJoins(glm::vec3, glm::vec3) = 0;
	virtual Link::CCollection candidateExtend(glm::vec3, glm::vec3) = 0;
	virtual Link::CCollection addStraight(glm::vec3, glm::vec3) = 0;
	virtual Link::CCollection addJoins(glm::vec3, glm::vec3) = 0;
	virtual Link::CCollection addExtend(glm::vec3, glm::vec3) = 0;

protected:
	static void joinLinks(const Link::Ptr & l, const Link::Ptr & ol);

	using Nodes = std::set<Node::Ptr, PtrMemberSorter<Node::Ptr, &Node::pos>>;
	Nodes nodes;
	std::shared_ptr<Texture> texture;
};

template<typename T> class NetworkOf : public Network, public Renderable {
protected:
	using Network::Network;

	Collection<T> links;
	void joinLinks(const Link::Ptr &) const;

protected:
	[[nodiscard]] Link::Ptr intersectRayLinks(const Ray &) const override;

public:
	template<typename L, typename... Params>
	std::shared_ptr<L>
	candidateLink(glm::vec3 a, glm::vec3 b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = candidateNodeAt(a).first, node2 = candidateNodeAt(b).first;
		return std::make_shared<L>(node1, node2, std::forward<Params>(params)...);
	}

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

	Link::CCollection candidateStraight(glm::vec3 n1, glm::vec3 n2) override;
	Link::CCollection candidateJoins(glm::vec3, glm::vec3) override;
	Link::CCollection candidateExtend(glm::vec3, glm::vec3) override;
	Link::CCollection addStraight(glm::vec3 n1, glm::vec3 n2) override;
	Link::CCollection addJoins(glm::vec3, glm::vec3) override;
	Link::CCollection addExtend(glm::vec3, glm::vec3) override;

	void render(const Shader &) const override;
};
