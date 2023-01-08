#include "network.h"
#include "routeWalker.h"
#include <array>
#include <cache.h>
#include <filesystem>
#include <game/network/link.h>
#include <gfx/models/texture.h>
#include <glm/gtx/intersect.hpp>
#include <initializer_list>
#include <ray.hpp>
#include <stdexcept>
#include <utility>

Network::Network(const std::string & tn) : texture {Texture::cachedTexture.get(tn)} { }

Node::Ptr
Network::nodeAt(glm::vec3 pos)
{
	return newNodeAt(pos).first;
}

Network::NodeInsertion
Network::newNodeAt(glm::vec3 pos)
{
	if (auto [n, i] = candidateNodeAt(pos); i == NodeIs::NotInNetwork) {
		return {*nodes.insert(std::move(n)).first, i};
	}
	else {
		return {std::move(n), NodeIs::InNetwork};
	}
}

Node::Ptr
Network::findNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return *n;
	}
	return {};
}

Network::NodeInsertion
Network::candidateNodeAt(glm::vec3 pos) const
{
	if (const auto n = nodes.find(pos); n != nodes.end()) {
		return {*n, NodeIs::InNetwork};
	}
	return {std::make_shared<Node>(pos), NodeIs::NotInNetwork};
}

Node::Ptr
Network::intersectRayNodes(const Ray & ray) const
{
	// Click within 2m of a node
	if (const auto node = std::find_if(nodes.begin(), nodes.end(),
				[&ray](const Node::Ptr & node) {
					glm::vec3 ipos, inorm;
					return glm::intersectRaySphere(ray.start, ray.direction, node->pos, 2.F, ipos, inorm);
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
Network::routeFromTo(const Link::End & start, glm::vec3 dest) const
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
Network::genCurveDef(const glm::vec3 & start, const glm::vec3 & end, float startDir)
{
	const auto diff {end - start};
	const auto vy {vector_yaw(diff)};
	const auto dir = pi + startDir;
	const auto flatStart {!start}, flatEnd {!end};
	const auto n2ed {(vy * 2) - dir - pi};
	const auto centre {find_arc_centre(flatStart, dir, flatEnd, n2ed)};

	if (centre.second) { // right hand arc
		return {end, start, centre.first};
	}
	return {start, end, centre.first};
}

std::pair<GenCurveDef, GenCurveDef>
Network::genCurveDef(const glm::vec3 & start, const glm::vec3 & end, float startDir, float endDir)
{
	startDir += pi;
	endDir += pi;
	const glm::vec2 flatStart {!start}, flatEnd {!end};
	auto midheight = [&](auto mid) {
		const auto sm = glm::distance(flatStart, mid), em = glm::distance(flatEnd, mid);
		return start.z + ((end.z - start.z) * (sm / (sm + em)));
	};
	if (const auto radii = find_arcs_radius(flatStart, startDir, flatEnd, endDir); radii.first < radii.second) {
		const auto radius {radii.first};
		const auto c1 = flatStart + sincosf(startDir + half_pi) * radius;
		const auto c2 = flatEnd + sincosf(endDir + half_pi) * radius;
		const auto mid = (c1 + c2) / 2.F;
		const auto midh = mid ^ midheight(mid);
		return {{start, midh, c1}, {end, midh, c2}};
	}
	else {
		const auto radius {radii.second};
		const auto c1 = flatStart + sincosf(startDir - half_pi) * radius;
		const auto c2 = flatEnd + sincosf(endDir - half_pi) * radius;
		const auto mid = (c1 + c2) / 2.F;
		const auto midh = mid ^ midheight(mid);
		return {{midh, start, c1}, {midh, end, c2}};
	}
}
