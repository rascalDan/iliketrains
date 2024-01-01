#pragma once

#include "link.h"
#include <collection.h>
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <sorting.h>
#include <special_members.h>
#include <string>
#include <utility>
#include <variant>

class Texture;
class SceneShader;
class Ray;

template<size_t... n> using GenDef = std::tuple<glm::vec<n, Distance>...>;
using GenCurveDef = GenDef<3, 3, 2>;

class Network {
public:
	using LinkEnd = std::pair<Link::Ptr, unsigned char>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;
	DEFAULT_MOVE_NO_COPY(Network);

	[[nodiscard]] Node::Ptr findNodeAt(Position3D) const;
	[[nodiscard]] Node::Ptr nodeAt(Position3D);
	enum class NodeIs { InNetwork, NotInNetwork };
	using NodeInsertion = std::pair<Node::Ptr, NodeIs>;
	[[nodiscard]] NodeInsertion newNodeAt(Position3D);
	[[nodiscard]] NodeInsertion candidateNodeAt(Position3D) const;
	[[nodiscard]] virtual Link::Ptr intersectRayLinks(const Ray &) const = 0;
	[[nodiscard]] virtual Node::Ptr intersectRayNodes(const Ray &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, Position3D) const;
	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, const Node::Ptr &) const;

	virtual Link::CCollection candidateStraight(Position3D, Position3D) = 0;
	virtual Link::CCollection candidateJoins(Position3D, Position3D) = 0;
	virtual Link::CCollection candidateExtend(Position3D, Position3D) = 0;
	virtual Link::CCollection addStraight(Position3D, Position3D) = 0;
	virtual Link::CCollection addJoins(Position3D, Position3D) = 0;
	virtual Link::CCollection addExtend(Position3D, Position3D) = 0;

	[[nodiscard]] virtual float findNodeDirection(Node::AnyCPtr) const = 0;

protected:
	static void joinLinks(const Link::Ptr & l, const Link::Ptr & ol);
	static GenCurveDef genCurveDef(const Position3D & start, const Position3D & end, float startDir);
	static std::pair<GenCurveDef, GenCurveDef> genCurveDef(
			const Position3D & start, const Position3D & end, float startDir, float endDir);

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
	candidateLink(Position3D a, Position3D b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = candidateNodeAt(a).first, node2 = candidateNodeAt(b).first;
		return std::make_shared<L>(node1, node2, std::forward<Params>(params)...);
	}

	template<typename L, typename... Params>
	std::shared_ptr<L>
	addLink(Position3D a, Position3D b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = nodeAt(a), node2 = nodeAt(b);
		auto l {links.template create<L>(node1, node2, std::forward<Params>(params)...)};
		joinLinks(l);
		return l;
	}

	Link::CCollection candidateStraight(Position3D n1, Position3D n2) override;
	Link::CCollection candidateJoins(Position3D, Position3D) override;
	Link::CCollection candidateExtend(Position3D, Position3D) override;
	Link::CCollection addStraight(Position3D n1, Position3D n2) override;
	Link::CCollection addJoins(Position3D, Position3D) override;
	Link::CCollection addExtend(Position3D, Position3D) override;

	[[nodiscard]] float findNodeDirection(Node::AnyCPtr) const override;

	void render(const SceneShader &) const override;

protected:
	Link::CCollection addJoins();
};
