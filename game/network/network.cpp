#include "network.h"
#include "routeWalker.h"
#include <array>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <glm/gtx/intersect.hpp>
#include <ray.h>
#include <stdexcept>
#include <utility>

Network::Network(const std::string & tn) :
	texture {std::make_shared<Texture>(tn,
			TextureOptions {
					.minFilter = GL_NEAREST_MIPMAP_LINEAR,
			})}
{
}

Node::Ptr
Network::nodeAt(GlobalPosition3D pos)
{
	return newNodeAt(pos).first;
}

Network::NodeInsertion
Network::newNodeAt(GlobalPosition3D pos)
{
	if (auto [n, i] = candidateNodeAt(pos); i == NodeIs::NotInNetwork) {
		return {*nodes.insert(std::move(n)).first, i};
	}
	else {
		return {std::move(n), NodeIs::InNetwork};
	}
}

Node::Ptr
Network::findNodeAt(GlobalPosition3D pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return *n;
	}
	return {};
}

Network::NodeInsertion
Network::candidateNodeAt(GlobalPosition3D pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return {*n, NodeIs::InNetwork};
	}
	return {std::make_shared<Node>(pos), NodeIs::NotInNetwork};
}

Node::Ptr
Network::intersectRayNodes(const Ray<GlobalPosition3D> & ray) const
{
	// Click within 2m of a node
	if (const auto node = std::find_if(nodes.begin(), nodes.end(),
				[&ray](const Node::Ptr & node) {
					GlobalPosition3D ipos;
					Normal3D inorm;
					return ray.intersectSphere(node->pos, 2000, ipos, inorm);
				});
			node != nodes.end()) {
		return *node;
	}
	return {};
}

void
Network::joinLinks(const Link::Ptr & l, const Link::Ptr & ol)
{
	if (l != ol) {
		for (const auto oe : {0U, 1U}) {
			for (const auto te : {0U, 1U}) {
				if (l->ends[te].node == ol->ends[oe].node) {
					l->ends[te].nexts.emplace_back(ol, oe);
					ol->ends[oe].nexts.emplace_back(l, te);
				}
			}
		}
	}
}

Link::Nexts
Network::routeFromTo(const Link::End & start, GlobalPosition3D dest) const
{
	auto destNode {findNodeAt(dest)};
	if (!destNode) {
		throw std::out_of_range("Node does not exist in network");
	}
	return routeFromTo(start, destNode);
}

Link::Nexts
Network::routeFromTo(const Link::End & end, const Node::Ptr & dest) const
{
	return RouteWalker().findRouteTo(end, dest);
}

GenCurveDef
Network::genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir)
{
	const auto diff = difference(end, start);
	const auto vy {vector_yaw(diff)};
	const auto dir = pi + startDir;
	const auto flatStart {start.xy()}, flatEnd {end.xy()};
	const auto n2ed {(vy * 2) - dir - pi};
	const auto centre {find_arc_centre(flatStart, dir, flatEnd, n2ed)};

	if (centre.second) { // right hand arc
		return {end, start, centre.first};
	}
	return {start, end, centre.first};
}

std::pair<GenCurveDef, GenCurveDef>
Network::genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir, float endDir)
{
	startDir += pi;
	endDir += pi;
	const auto flatStart {start.xy()}, flatEnd {end.xy()};
	auto midheight = [&](auto mid) {
		const auto sm = glm::length(RelativePosition2D(flatStart - mid)),
				   em = glm::length(RelativePosition2D(flatEnd - mid));
		return start.z + GlobalDistance(RelativeDistance(end.z - start.z) * (sm / (sm + em)));
	};
	if (const auto radii = find_arcs_radius(flatStart, startDir, flatEnd, endDir); radii.first < radii.second) {
		const auto radius {radii.first};
		const auto c1 = flatStart + (sincos(startDir + half_pi) * radius);
		const auto c2 = flatEnd + (sincos(endDir + half_pi) * radius);
		const auto mid = (c1 + c2) / 2;
		const auto midh = mid || midheight(mid);
		return {{start, midh, c1}, {end, midh, c2}};
	}
	else {
		const auto radius {radii.second};
		const auto c1 = flatStart + (sincos(startDir - half_pi) * radius);
		const auto c2 = flatEnd + (sincos(endDir - half_pi) * radius);
		const auto mid = (c1 + c2) / 2;
		const auto midh = mid || midheight(mid);
		return {{midh, start, c1}, {midh, end, c2}};
	}
}
