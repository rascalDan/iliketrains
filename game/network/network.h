#pragma once

#include "collection.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"
#include "link.h"
#include "sorting.h"
#include "special_members.h"
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <string>
#include <utility>

class SceneShader;
class Surface;
template<typename> class Ray;

template<size_t... n> using GenDef = std::tuple<glm::vec<n, GlobalDistance>...>;
using GenCurveDef = GenDef<3, 3, 2>;

class Network {
public:
	using LinkEnd = std::pair<Link::Ptr, unsigned char>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;
	DEFAULT_MOVE_NO_COPY(Network);

	[[nodiscard]] Node::Ptr findNodeAt(GlobalPosition3D) const;
	[[nodiscard]] Node::Ptr nodeAt(GlobalPosition3D);
	enum class NodeIs { InNetwork, NotInNetwork };
	using NodeInsertion = std::pair<Node::Ptr, NodeIs>;
	[[nodiscard]] NodeInsertion newNodeAt(GlobalPosition3D);
	[[nodiscard]] NodeInsertion candidateNodeAt(GlobalPosition3D) const;
	[[nodiscard]] virtual Link::Ptr intersectRayLinks(const Ray<GlobalPosition3D> &) const = 0;
	[[nodiscard]] virtual Node::Ptr intersectRayNodes(const Ray<GlobalPosition3D> &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, GlobalPosition3D) const;
	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, const Node::Ptr &) const;

	virtual Link::CCollection candidateStraight(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection candidateJoins(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection candidateExtend(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addStraight(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addJoins(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addExtend(GlobalPosition3D, GlobalPosition3D) = 0;

	[[nodiscard]] virtual float findNodeDirection(Node::AnyCPtr) const = 0;

	[[nodiscard]] virtual const Surface * getBaseSurface() const = 0;
	[[nodiscard]] virtual RelativeDistance getBaseWidth() const = 0;

protected:
	static void joinLinks(const Link::Ptr & l, const Link::Ptr & ol);
	static GenCurveDef genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir);
	static std::pair<GenCurveDef, GenCurveDef> genCurveDef(
			const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir, float endDir);

	using Nodes = std::set<Node::Ptr, PtrMemberSorter<Node::Ptr, &Node::pos>>;
	Nodes nodes;
	Texture::Ptr texture;
};

template<typename LinkType> class NetworkLinkHolder {
public:
	// Implemented per LinkType to configure vao
	NetworkLinkHolder();
	friend LinkType;
	glVertexArray vao;
	mutable InstanceVertices<typename LinkType::Vertex> vertices;
};

template<typename T, typename... Links>
class NetworkOf : public Network, public Renderable, public NetworkLinkHolder<Links>... {
protected:
	using Network::Network;

	Collection<T> links;
	void joinLinks(const Link::Ptr &) const;

protected:
	[[nodiscard]] Link::Ptr intersectRayLinks(const Ray<GlobalPosition3D> &) const override;

public:
	template<typename L, typename... Params>
	std::shared_ptr<L>
	candidateLink(GlobalPosition3D a, GlobalPosition3D b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = candidateNodeAt(a).first, node2 = candidateNodeAt(b).first;
		return std::make_shared<L>(*this, node1, node2, std::forward<Params>(params)...);
	}

	template<typename L, typename... Params>
	std::shared_ptr<L>
	addLink(GlobalPosition3D a, GlobalPosition3D b, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = nodeAt(a), node2 = nodeAt(b);
		auto l {links.template create<L>(*this, node1, node2, std::forward<Params>(params)...)};
		joinLinks(l);
		return l;
	}

	Link::CCollection candidateStraight(GlobalPosition3D n1, GlobalPosition3D n2) override;
	Link::CCollection candidateJoins(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection candidateExtend(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection addStraight(GlobalPosition3D n1, GlobalPosition3D n2) override;
	Link::CCollection addJoins(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection addExtend(GlobalPosition3D, GlobalPosition3D) override;

	[[nodiscard]] float findNodeDirection(Node::AnyCPtr) const override;

protected:
	Link::CCollection addJoins();
};
