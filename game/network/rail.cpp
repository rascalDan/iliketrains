#include "rail.h"
#include "game/gamestate.h"
#include "game/geoData.h"
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
			const auto sm = ::distance<2>(flatStart, mid);
			const auto em = ::distance<2>(flatEnd, mid);
			return start.z + GlobalDistance(RelativeDistance(end.z - start.z) * (sm / (sm + em)));
		};
		const float dir2 = pi + findNodeDirection(node2ins.first);
		if (const auto radii = find_arcs_radius(flatStart, dir, flatEnd, dir2); radii.first < radii.second) {
			const auto radius {radii.first};
			const auto c1 = flatStart + (sincos(dir + half_pi) * radius);
			const auto c2 = flatEnd + (sincos(dir2 + half_pi) * radius);
			const auto mid = (c1 + c2) / 2;
			const auto midh = mid || midheight(mid);
			addLink<RailLinkCurve>(start, midh, c1);
			return addLink<RailLinkCurve>(end, midh, c2);
		}
		else {
			const auto radius {radii.second};
			const auto c1 = flatStart + (sincos(dir - half_pi) * radius);
			const auto c2 = flatEnd + (sincos(dir2 - half_pi) * radius);
			const auto mid = (c1 + c2) / 2;
			const auto midh = mid || midheight(mid);
			addLink<RailLinkCurve>(midh, start, c1);
			return addLink<RailLinkCurve>(midh, end, c2);
		}
	}
	const auto diff = difference(end, start);
	const auto vy {vector_yaw(diff)};
	const auto n2ed {(vy * 2) - dir - pi};
	const auto centre {find_arc_centre(flatStart, dir, flatEnd, n2ed)};

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

RailLinkStraight::RailLinkStraight(NetworkLinkHolder<RailLinkStraight> & instances, const Node::Ptr & a,
		const Node::Ptr & b) : RailLinkStraight(instances, a, b, b->pos - a->pos)
{
}

RailLinkStraight::RailLinkStraight(
		NetworkLinkHolder<RailLinkStraight> & instances, Node::Ptr a, Node::Ptr b, const RelativePosition3D & diff) :
	Link({std::move(a), vector_yaw(diff)}, {std::move(b), vector_yaw(-diff)}, glm::length(diff)),
	instance {instances.vertices.acquire(
			ends[0].node->pos, ends[1].node->pos, flat_orientation(diff), roundSleepers(length))}
{
}

RailLinkCurve::RailLinkCurve(
		NetworkLinkHolder<RailLinkCurve> & instances, const Node::Ptr & a, const Node::Ptr & b, GlobalPosition2D c) :
	RailLinkCurve(instances, a, b, c || a->pos.z, ::distance<2>(a->pos.xy(), c), {c, a->pos, b->pos})
{
}

RailLinkCurve::RailLinkCurve(NetworkLinkHolder<RailLinkCurve> & instances, const Node::Ptr & a, const Node::Ptr & b,
		GlobalPosition3D c, RelativeDistance radius, const Arc arc) :
	Link({a, normalize(arc.first + half_pi)}, {b, normalize(arc.second - half_pi)},
			glm::length(RelativePosition2D {radius * arc.length(), difference(a->pos, b->pos).z})),
	LinkCurve {c, radius, arc}, instance {instances.vertices.acquire(ends[0].node->pos, ends[1].node->pos, c,
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
	renderType(const NetworkLinkHolder<LinkType> & n, auto & s)
	{
		if (auto count = n.vertices.size()) {
			s.use(RAIL_CROSS_SECTION, RAIL_TEXTURE_POS);
			glBindVertexArray(n.vao);
			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
		}
	};
}

void
RailLinks::render(const SceneShader & shader) const
{
	if (!links.objects.empty()) {
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
	return std::dynamic_pointer_cast<Surface>(gameState->assets.at("terrain.surface.gravel")).get();
}

RelativeDistance
RailLinks::getBaseWidth() const
{
	return 5'700;
}
