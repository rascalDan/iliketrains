#pragma once

#include "collection.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"
#include "link.h"
#include "sorting.h"
#include "special_members.h"
#include <glm/glm.hpp>
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

	[[nodiscard]] virtual float findNodeDirection(Node::AnyCPtr) const = 0;

	[[nodiscard]] Link::Collection create(const GeoData *, const CreationDefinition &);
	[[nodiscard]] Link::Collection createChain(const GeoData *, std::span<const GlobalPosition3D>);
	virtual void add(GeoData *, const Link::Ptr &) = 0;
	void add(GeoData *, std::span<const Link::Ptr>);

	[[nodiscard]] virtual const Surface * getBaseSurface() const = 0;
	[[nodiscard]] virtual RelativeDistance getBaseWidth() const = 0;

protected:
	static void joinLinks(const Link::Ptr & link, const Link::Ptr & oldLink);
	static GenCurveDef genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir);
	static std::pair<GenCurveDef, GenCurveDef> genCurveDef(
			const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir, float endDir);

	[[nodiscard]] GenLinksDef terrainSplit(const GeoData *, const GenStraightDef &) const;
	[[nodiscard]] GenLinksDef terrainSplit(const GeoData *, const GenCurveDef &) const;
	static void terrainSplitAt(GenLinkDef & previous, GenLinkDef & next, GlobalPosition3D pos);
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
	[[nodiscard]] float findNodeDirection(Node::AnyCPtr) const override;
	using Network::create;
	[[nodiscard]] Link::Ptr create(const GenStraightDef &) override;
	[[nodiscard]] Link::Ptr create(const GenCurveDef &) override;
	using Network::add;
	void add(GeoData *, const Link::Ptr &) override;

protected:
	[[nodiscard]] bool anyLinks() const;
};
