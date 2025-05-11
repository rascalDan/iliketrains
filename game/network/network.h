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
#include <variant>
#include <vector>

class SceneShader;
struct Surface;
class GeoData;
template<typename> class Ray;

template<size_t... N> using GenDef = std::tuple<glm::vec<N, GlobalDistance>...>;
using GenStraightDef = GenDef<3, 3>;
using GenCurveDef = GenDef<3, 3, 2>;
using GenLinkDef = std::variant<GenStraightDef, GenCurveDef>;
using GenLinksDef = std::vector<GenLinkDef>;

struct CreationDefinitionEnd {
	GlobalPosition3D position;
	std::optional<Angle> direction;
};

struct CreationDefinition {
	CreationDefinitionEnd fromEnd;
	CreationDefinitionEnd toEnd;
};

class Network {
public:
	using LinkEnd = std::pair<Link::Ptr, unsigned char>;
	explicit Network(const std::string & textureName);
	virtual ~Network() = default;
	DEFAULT_MOVE_NO_COPY(Network);

	[[nodiscard]] Node::Ptr findNodeAt(GlobalPosition3D) const;
	[[nodiscard]] Node::Ptr nodeAt(GlobalPosition3D);
	enum class NodeIs : uint8_t { InNetwork, NotInNetwork };
	using NodeInsertion = std::pair<Node::Ptr, NodeIs>;
	[[nodiscard]] NodeInsertion newNodeAt(GlobalPosition3D);
	[[nodiscard]] NodeInsertion candidateNodeAt(GlobalPosition3D) const;
	[[nodiscard]] virtual Link::Ptr intersectRayLinks(const Ray<GlobalPosition3D> &) const = 0;
	[[nodiscard]] virtual Node::Ptr intersectRayNodes(const Ray<GlobalPosition3D> &) const;

	[[nodiscard]] Link::Nexts routeFromTo(const Link::End &, GlobalPosition3D) const;
	[[nodiscard]] static Link::Nexts routeFromTo(const Link::End &, const Node::Ptr &);

	virtual Link::CCollection candidateStraight(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection candidateJoins(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection candidateExtend(GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addStraight(const GeoData *, GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addJoins(const GeoData *, GlobalPosition3D, GlobalPosition3D) = 0;
	virtual Link::CCollection addExtend(const GeoData *, GlobalPosition3D, GlobalPosition3D) = 0;

	[[nodiscard]] virtual float findNodeDirection(Node::AnyCPtr) const = 0;

	[[nodiscard]] Link::Collection create(const CreationDefinition &);
	virtual void add(GeoData *, const Link::Ptr &) = 0;

	[[nodiscard]] virtual const Surface * getBaseSurface() const = 0;
	[[nodiscard]] virtual RelativeDistance getBaseWidth() const = 0;

protected:
	static void joinLinks(const Link::Ptr & link, const Link::Ptr & oldLink);
	static GenCurveDef genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir);
	static std::pair<GenCurveDef, GenCurveDef> genCurveDef(
			const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir, float endDir);

	[[nodiscard]] virtual Link::Ptr create(const GenStraightDef &) = 0;
	[[nodiscard]] virtual Link::Ptr create(const GenCurveDef &) = 0;

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

	SharedCollection<T> links;
	void joinLinks(const Link::Ptr &) const;

	[[nodiscard]] Link::Ptr intersectRayLinks(const Ray<GlobalPosition3D> &) const override;

public:
	template<typename L, typename... Params>
	std::shared_ptr<L>
	candidateLink(GlobalPosition3D positionA, GlobalPosition3D positionB, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = candidateNodeAt(positionA).first, node2 = candidateNodeAt(positionB).first;
		return std::make_shared<L>(*this, node1, node2, std::forward<Params>(params)...);
	}

	template<typename L, typename... Params>
	std::shared_ptr<L>
	addLink(GlobalPosition3D positionA, GlobalPosition3D positionB, Params &&... params)
		requires std::is_base_of_v<T, L>
	{
		const auto node1 = nodeAt(positionA), node2 = nodeAt(positionB);
		auto newLink = links.template create<L>(*this, node1, node2, std::forward<Params>(params)...);
		joinLinks(newLink);
		return std::move(newLink);
	}

	Link::CCollection candidateStraight(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection candidateJoins(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection candidateExtend(GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection addStraight(const GeoData *, GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection addJoins(const GeoData *, GlobalPosition3D, GlobalPosition3D) override;
	Link::CCollection addExtend(const GeoData *, GlobalPosition3D, GlobalPosition3D) override;

	[[nodiscard]] float findNodeDirection(Node::AnyCPtr) const override;
	using Network::create;
	[[nodiscard]] Link::Ptr create(const GenStraightDef &) override;
	[[nodiscard]] Link::Ptr create(const GenCurveDef &) override;
	void add(GeoData *, const Link::Ptr &) override;

protected:
	Link::CCollection addCurve(const GeoData *, const GenCurveDef &);
	[[nodiscard]] bool anyLinks() const;
};
