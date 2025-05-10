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
					return ray.intersectSphere(node->pos, MIN_DISTANCE);
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
	const auto dir = pi + startDir;
	const auto flatStart = start.xy(), flatEnd = end.xy();
	const auto centre = find_arc_centre(flatStart, dir, flatEnd);

	if (centre.second) { // right hand arc
		return {end, start, centre.first};
	}
	return {start, end, centre.first};
}

std::pair<GenCurveDef, GenCurveDef>
Network::genCurveDef(const GlobalPosition3D & start, const GlobalPosition3D & end, float startDir, float endDir)
{
	// Based on formula/code from https://www.ryanjuckett.com/biarc-interpolation/
	const auto startVec = -sincos(startDir);
	const auto endVec = sincos(endDir);
	const auto diff = difference(end, start);
	const auto diffDotStartVec = glm::dot(diff.xy(), startVec);
	const auto endsVecTotal = (startVec + endVec);
	const auto tMagSqr = vectorMagSquared(endsVecTotal);
	const auto equalTangents = isWithinLimit(tMagSqr, 4.0F);
	const auto perpT1 = isWithinLimit(diffDotStartVec, 0.0F);

	if (equalTangents && perpT1) {
		const auto joint = start + (diff * 0.5F);
		return {genCurveDef(start, joint, startDir), genCurveDef(end, joint, endDir)};
	}

	const auto vDotT = glm::dot(diff.xy(), endsVecTotal);
	const auto extLen1 = [&]() {
		const auto vMagSqr = vectorMagSquared(diff);
		if (equalTangents) {
			return vMagSqr / (4 * diffDotStartVec);
		}
		const auto denominator = 2.F - (2.F * glm::dot(startVec, endVec));
		const auto discriminant = sq(vDotT) + (denominator * vMagSqr);
		return (std::sqrt(discriminant) - vDotT) / denominator;
	}();

	const auto joint = (start + end + ((difference(startVec, endVec) * extLen1) || 0.F)) / 2;

	return {genCurveDef(start, joint, startDir), genCurveDef(end, joint, endDir)};
}

Link::Collection
Network::create(const CreationDefinition & def)
{
	// TODO
	// Where to make a straight to join because angles align?
	// Where to drop part of S curve pair if a single curve works?

	if (!def.fromEnd.direction && !def.toEnd.direction) {
		// No specific directions at either end, straight link
		return {create(GenStraightDef {def.fromEnd.position, def.toEnd.position})};
	}
	if (def.fromEnd.direction) {
		if (def.toEnd.direction) {
			// Two specific directions at both ends, S curves
			const auto curves = genCurveDef(
					def.fromEnd.position, def.toEnd.position, *def.fromEnd.direction, *def.toEnd.direction);
			return {create(curves.first), create(curves.second)};
		}
		// One specific direction, single curve from there
		return {create(genCurveDef(def.fromEnd.position, def.toEnd.position, *def.fromEnd.direction))};
	}
	// One specific direction, single curve from the other
	return {create(genCurveDef(def.toEnd.position, def.fromEnd.position, *def.toEnd.direction))};
}
