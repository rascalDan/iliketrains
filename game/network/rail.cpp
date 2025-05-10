#include "rail.h"
#include "game/gamestate.h"
#include "network.h"
#include <game/network/network.impl.h> // IWYU pragma: keep
#include <gfx/gl/sceneShader.h>
#include <gfx/gl/vertexArrayObject.h>
#include <gfx/models/texture.h>

template class NetworkOf<RailLink, RailLinkStraight, RailLinkCurve>;

constexpr auto RAIL_CROSSSECTION_VERTICES {5U};
constexpr Size3D RAIL_HEIGHT {0, 0, 50.F};

RailLinks::RailLinks() : NetworkOf<RailLink, RailLinkStraight, RailLinkCurve> {"rails.jpg"} { }

void
RailLinks::tick(TickDuration)
{
}

std::shared_ptr<RailLink>
RailLinks::addLinksBetween(GlobalPosition3D start, GlobalPosition3D end)
{
	auto node1ins = newNodeAt(start), node2ins = newNodeAt(end);
	if (node1ins.second == NodeIs::NotInNetwork && node2ins.second == NodeIs::NotInNetwork) {
		// Both nodes are new, direct link, easy
		return addLink<RailLinkStraight>(start, end);
	}
	if (node1ins.second == NodeIs::NotInNetwork && node2ins.second == NodeIs::InNetwork) {
		// node1 is new, node2 exists, but we build from existing outwards
		std::swap(node1ins, node2ins);
		std::swap(start, end);
	}
	// Find start link/end - opposite entry dir to existing link; so pi +...
	const Angle dir = pi + findNodeDirection(node1ins.first);
	if (dir == vector_yaw(difference(end, start))) {
		return addLink<RailLinkStraight>(start, end);
	}
	const auto flatStart {start.xy()}, flatEnd {end.xy()};
	if (node2ins.second == NodeIs::InNetwork) {
		auto midheight = [&](auto mid) {
			const auto startToMid = ::distance<2>(flatStart, mid);
			const auto endToMid = ::distance<2>(flatEnd, mid);
			return start.z + GlobalDistance(RelativeDistance(end.z - start.z) * (startToMid / (startToMid + endToMid)));
		};
		const float dir2 = pi + findNodeDirection(node2ins.first);
		const auto radii = find_arcs_radius(flatStart, dir, flatEnd, dir2);
		if (radii.first < radii.second) {
			const auto radius = radii.first;
			const auto centre1 = flatStart + (sincos(dir + half_pi) * radius);
			const auto centre2 = flatEnd + (sincos(dir2 + half_pi) * radius);
			const auto mid = (centre1 + centre2) / 2;
			const auto midh = mid || midheight(mid);
			addLink<RailLinkCurve>(start, midh, centre1);
			return addLink<RailLinkCurve>(end, midh, centre2);
		}
		const auto radius = radii.second;
		const auto centre1 = flatStart + (sincos(dir - half_pi) * radius);
		const auto centre2 = flatEnd + (sincos(dir2 - half_pi) * radius);
		const auto mid = (centre1 + centre2) / 2;
		const auto midh = mid || midheight(mid);
		addLink<RailLinkCurve>(midh, start, centre1);
		return addLink<RailLinkCurve>(midh, end, centre2);
	}
	const auto diff = difference(end, start);
	const auto yaw = vector_yaw(diff);
	const auto n2ed = (yaw * 2) - dir - pi;
	const auto centre = find_arc_centre(flatStart, dir, flatEnd, n2ed);

	if (centre.second) { // right hand arc
		std::swap(start, end);
	}
	return addLink<RailLinkCurve>(start, end, centre.first);
}

namespace {
	constexpr const std::array<RelativePosition3D, RAIL_CROSSSECTION_VERTICES> RAIL_CROSS_SECTION {{
			{-1330.F, 0.F, 0},
			{-608.F, 0.F, RAIL_HEIGHT.z},
			{0, 0.F, RAIL_HEIGHT.z / 2},
			{608.F, 0.F, RAIL_HEIGHT.z},
			{1330.F, 0.F, 0},
	}};
	constexpr const std::array<float, RAIL_CROSSSECTION_VERTICES> RAIL_TEXTURE_POS {
			0.15F,
			.34F,
			.5F,
			.66F,
			0.85F,
	};
	template<std::floating_point T> constexpr T SLEEPERS_PER_TEXTURE {5};
	template<std::floating_point T> constexpr T TEXTURE_LENGTH {2'000};
	template<std::floating_point T> constexpr T SLEEPER_LENGTH {T {1} / SLEEPERS_PER_TEXTURE<T>};

	template<std::floating_point T>
	constexpr auto
	roundSleepers(const T length)
	{
		return round_frac(length / TEXTURE_LENGTH<T>, SLEEPER_LENGTH<T>);
	}
}

RailLinkStraight::RailLinkStraight(NetworkLinkHolder<RailLinkStraight> & instances, const Node::Ptr & nodeA,
		const Node::Ptr & nodeB) : RailLinkStraight(instances, nodeA, nodeB, nodeB->pos - nodeA->pos)
{
}

RailLinkStraight::RailLinkStraight(NetworkLinkHolder<RailLinkStraight> & instances, Node::Ptr nodeA, Node::Ptr nodeB,
		const RelativePosition3D & diff) :
	Link({.node = std::move(nodeA), .dir = vector_yaw(diff)}, {.node = std::move(nodeB), .dir = vector_yaw(-diff)},
			glm::length(diff)),
	instance {instances.vertices.acquire(
			ends[0].node->pos, ends[1].node->pos, flat_orientation(diff), roundSleepers(length))}
{
}

RailLinkCurve::RailLinkCurve(NetworkLinkHolder<RailLinkCurve> & instances, const Node::Ptr & nodeA,
		const Node::Ptr & nodeB, GlobalPosition2D centre) :
	RailLinkCurve(instances, nodeA, nodeB, centre || nodeA->pos.z, ::distance<2>(nodeA->pos.xy(), centre),
			{centre, nodeA->pos, nodeB->pos})
{
}

RailLinkCurve::RailLinkCurve(NetworkLinkHolder<RailLinkCurve> & instances, const Node::Ptr & nodeA,
		const Node::Ptr & nodeB, GlobalPosition3D centre, RelativeDistance radius, const Arc arc) :
	Link({.node = nodeA, .dir = normalize(arc.first + half_pi)},
			{.node = nodeB, .dir = normalize(arc.second - half_pi)},
			glm::length(RelativePosition2D {radius * arc.length(), difference(nodeA->pos, nodeB->pos).z})),
	LinkCurve {centre, radius, arc}, instance {instances.vertices.acquire(ends[0].node->pos, ends[1].node->pos, centre,
											 roundSleepers(length), half_pi - arc.first, half_pi - arc.second, radius)}
{
}

RelativePosition3D
RailLink::vehiclePositionOffset() const
{
	return RAIL_HEIGHT;
}

template<> NetworkLinkHolder<RailLinkStraight>::NetworkLinkHolder()
{
	VertexArrayObject {vao}
			.addAttribs<RailLinkStraight::Vertex, &RailLinkStraight::Vertex::a, &RailLinkStraight::Vertex::b,
					&RailLinkStraight::Vertex::rotation, &RailLinkStraight::Vertex::textureRepeats>(
					vertices.bufferName());
}

template<> NetworkLinkHolder<RailLinkCurve>::NetworkLinkHolder()
{
	VertexArrayObject {vao}
			.addAttribs<RailLinkCurve::Vertex, &RailLinkCurve::Vertex::a, &RailLinkCurve::Vertex::b,
					&RailLinkCurve::Vertex::c, &RailLinkCurve::Vertex::textureRepeats, &RailLinkCurve::Vertex::aangle,
					&RailLinkCurve::Vertex::bangle, &RailLinkCurve::Vertex::radius>(vertices.bufferName());
}

namespace {
	template<typename LinkType>
	void
	renderType(const NetworkLinkHolder<LinkType> & networkLinks, auto & shader)
	{
		if (auto count = networkLinks.vertices.size()) {
			shader.use(RAIL_CROSS_SECTION, RAIL_TEXTURE_POS);
			glBindVertexArray(networkLinks.vao);
			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
		}
	};
}

void
RailLinks::render(const SceneShader & shader, const Frustum &) const
{
	if (anyLinks()) {
		texture->bind();
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1, 0);
		renderType<RailLinkStraight>(*this, shader.networkStraight);
		renderType<RailLinkCurve>(*this, shader.networkCurve);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindVertexArray(0);
	}
}

const Surface *
RailLinks::getBaseSurface() const
{
	return gameState->assets.at("terrain.surface.gravel").dynamicCast<const Surface>().get();
}

RelativeDistance
RailLinks::getBaseWidth() const
{
	static constexpr auto BASE_WIDTH = 5'700;
	return BASE_WIDTH;
}
