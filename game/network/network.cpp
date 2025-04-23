#include "network.h"
#include "routeWalker.h"
#include <array>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <glm/gtx/intersect.hpp>
#include <ray.h>
#include <stdexcept>
#include <utility>

Network::Network(const std::string & textureName) :
	texture {std::make_shared<Texture>(textureName,
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
	auto [node, inNetwork] = candidateNodeAt(pos);
	if (inNetwork == NodeIs::NotInNetwork) {
		return {*nodes.insert(std::move(node)).first, inNetwork};
	}
	return {std::move(node), NodeIs::InNetwork};
}

Node::Ptr
Network::findNodeAt(GlobalPosition3D pos) const
{
	if (const auto node = nodes.find(pos); node != nodes.end()) {
		return *node;
	}
	return {};
}

Network::NodeInsertion
Network::candidateNodeAt(GlobalPosition3D pos) const
{
	if (const auto node = nodes.find(pos); node != nodes.end()) {
		return {*node, NodeIs::InNetwork};
	}
	return {std::make_shared<Node>(pos), NodeIs::NotInNetwork};
}

Node::Ptr
Network::intersectRayNodes(const Ray<GlobalPosition3D> & ray) const
{
	static constexpr auto MIN_DISTANCE = 2000;
	// Click within 2m of a node
	if (const auto node = std::find_if(nodes.begin(), nodes.end(),
				[&ray](const Node::Ptr & node) {
					GlobalPosition3D ipos;
					Normal3D inorm;
					return ray.intersectSphere(node->pos, MIN_DISTANCE, ipos, inorm);
				});
			node != nodes.end()) {
		return *node;
	}
	return {};
}

void
Network::joinLinks(const Link::Ptr & link, const Link::Ptr & oldLink)
{
	if (link != oldLink) {
		for (const auto oldLinkEnd : {0U, 1U}) {
			for (const auto linkEnd : {0U, 1U}) {
				if (link->ends[linkEnd].node == oldLink->ends[oldLinkEnd].node) {
					link->ends[linkEnd].nexts.emplace_back(oldLink, oldLinkEnd);
					oldLink->ends[oldLinkEnd].nexts.emplace_back(link, linkEnd);
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
Network::routeFromTo(const Link::End & end, const Node::Ptr & dest)
{
	return RouteWalker().findRouteTo(end, dest);
}

GenCurveDef
Network::genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir)
{
	const auto diff = difference(end, start);
	const auto yaw = vector_yaw(diff);
	const auto dir = pi + startDir;
	const auto flatStart = start.xy(), flatEnd = end.xy();
	const auto n2ed = (yaw * 2) - dir - pi;
	const auto centre = find_arc_centre(flatStart, dir, flatEnd, n2ed);

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
		const auto startToMid = ::distance<2>(flatStart, mid);
		const auto endToMid = ::distance<2>(flatEnd, mid);
		return start.z + GlobalDistance(RelativeDistance(end.z - start.z) * (startToMid / (startToMid + endToMid)));
	};
	const auto radii = find_arcs_radius(flatStart, startDir, flatEnd, endDir);
	if (radii.first < radii.second) {
		const auto radius = radii.first;
		const auto centre1 = flatStart + (sincos(startDir + half_pi) * radius);
		const auto centre2 = flatEnd + (sincos(endDir + half_pi) * radius);
		const auto mid = (centre1 + centre2) / 2;
		const auto midh = mid || midheight(mid);
		return {{start, midh, centre1}, {end, midh, centre2}};
	}
	const auto radius = radii.second;
	const auto centre1 = flatStart + (sincos(startDir - half_pi) * radius);
	const auto centre2 = flatEnd + (sincos(endDir - half_pi) * radius);
	const auto mid = (centre1 + centre2) / 2;
	const auto midh = mid || midheight(mid);
	return {{midh, start, centre1}, {midh, end, centre2}};
}
