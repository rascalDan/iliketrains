#include "network.h"
#include "game/geoData.h"
#include "routeWalker.h"
#include <array>
#include <collections.h>
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

void
Network::add(GeoData * geoData, const std::span<const Link::Ptr> links)
{
	for (const auto & link : links) {
		add(geoData, link);
	}
}

void
Network::connectAt(GenLinkDef & previous, GenLinkDef & next, GlobalPosition3D pos)
{
	std::visit(
			[pos](auto & typedDefPrevious, auto & typedDefNext) {
				std::get<1>(typedDefPrevious) = std::get<0>(typedDefNext) = pos;
			},
			previous, next);
}

GenLinksDef
Network::terrainSplit(const GeoData * geoData, const GenStraightDef & def) const
{
	GenLinksDef out {def};
	const auto [fromPos, toPos] = def;
	geoData->walk(fromPos.xy(), toPos, [geoData, &out](const GeoData::WalkStep & step) {
		if (step.previous.is_valid() && geoData->getSurface(step.current) != geoData->getSurface(step.previous)) {
			const auto surfaceEdgePosition = geoData->positionAt(GeoData::PointFace(step.exitPosition, step.current));
			out.emplace_back(out.back());
			connectAt(*out.rbegin(), *++out.rbegin(), surfaceEdgePosition);
		}
	});
	return out;
}

GenLinksDef
Network::terrainSplit(const GeoData * geoData, const GenCurveDef & def) const
{
	static constexpr auto MIN_DISTANCE = 2000.F;
	auto [cstart, cend, centre] = def;
	std::set<GeoData::WalkStepCurve, SortedBy<&GeoData::WalkStepCurve::angle>> breaks;
	const auto radiusMid = ::distance(cstart.xy(), centre);
	for (const auto radiusOffset : {-getBaseWidth() / 2.F, 0.F, getBaseWidth() / 2.F}) {
		const auto radius = radiusOffset + radiusMid;
		const auto start = centre + (difference(cstart.xy(), centre) * radius) / radiusMid;
		const auto end = centre + (difference(cend.xy(), centre) * radius) / radiusMid;
		geoData->walk(start, end, centre, [geoData, &breaks](const GeoData::WalkStepCurve & step) {
			if (step.previous.is_valid() && geoData->getSurface(step.current) != geoData->getSurface(step.previous)) {
				breaks.insert(step);
			}
		});
	}
	std::vector<GlobalPosition3D> points;
	points.reserve(breaks.size() + 2);
	points.push_back(cstart);
	std::ranges::transform(
			breaks, std::back_inserter(points), [geoData, centre, radiusMid](const GeoData::WalkStepCurve & step) {
				return (centre + (sincos(step.angle) * radiusMid))
						|| geoData->positionAt(GeoData::PointFace(step.exitPosition, step.current)).z;
			});
	points.push_back(cend);
	mergeClose(points, ::distance<3, GlobalDistance>, ::midpoint<3, GlobalDistance>, MIN_DISTANCE);
	GenLinksDef out {def};
	std::ranges::for_each(++points.begin(), --points.end(), [&out](const auto pos) {
		out.emplace_back(out.back());
		connectAt(*out.rbegin(), *++out.rbegin(), pos);
	});
	return out;
}

GenLinksDef
Network::genDef(const GlobalPosition3D & start, const GlobalPosition3D & end)
{
	return {GenStraightDef {start, end}};
}

GenLinksDef
Network::genDef(const GlobalPosition3D & start, const GlobalPosition3D & end, Angle startDir)
{
	const auto dir = pi + startDir;
	const auto flatStart = start.xy(), flatEnd = end.xy();
	const auto centre = find_arc_centre(flatStart, dir, flatEnd);

	if (centre.second) { // right hand arc
		return {GenCurveDef {end, start, centre.first}};
	}
	return {GenCurveDef {start, end, centre.first}};
}

GenLinksDef
Network::genDef(const GlobalPosition3D & start, const GlobalPosition3D & end, Angle startDir, Angle endDir)
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
		return genDef(start, joint, startDir) + genDef(end, joint, endDir);
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

	return genDef(start, joint, startDir) + genDef(end, joint, endDir);
}

Link::Collection
Network::create(const GeoData * geoData, const CreationDefinition & def)
{
	// TODO
	// Where to make a straight to join because angles align?
	// Where to drop part of S curve pair if a single curve works?

	const auto linkDefs = [&def]() -> GenLinksDef {
		if (!def.fromEnd.direction && !def.toEnd.direction) {
			// No specific directions at either end, straight link
			return {GenStraightDef {def.fromEnd.position, def.toEnd.position}};
		}
		if (def.fromEnd.direction) {
			if (def.toEnd.direction) {
				// Two specific directions at both ends, S curves
				return genDef(def.fromEnd.position, def.toEnd.position, *def.fromEnd.direction, *def.toEnd.direction);
			}
			// One specific direction, single curve from there
			return genDef(def.fromEnd.position, def.toEnd.position, *def.fromEnd.direction);
		}
		// One specific direction, single curve from the other
		return genDef(def.toEnd.position, def.fromEnd.position, *def.toEnd.direction);
	};
	const auto splitDefs = [&linkDefs, this, geoData]() {
		return std::ranges::fold_left(linkDefs(), GenLinksDef {}, [this, geoData](auto && existing, const auto & def) {
			return existing += std::visit(
						   [this, geoData](const auto & typedDef) {
							   return this->terrainSplit(geoData, typedDef);
						   },
						   def);
		});
	};
	Link::Collection links;
	std::ranges::transform(geoData ? splitDefs() : linkDefs(), std::back_inserter(links), [this](const auto & def) {
		return std::visit(
				[this](const auto & typedDef) {
					return this->create(typedDef);
				},
				def);
	});
	return links;
}

Link::Collection
Network::createChain(const GeoData * geoData, const std::span<const GlobalPosition3D> positions)
{
	Link::Collection out;
	std::ranges::for_each(positions | std::views::pairwise,
			[&out, dir = std::optional<Angle> {}, geoData, this](const auto & current) mutable {
				const auto & [previous, next] = current;
				CreationDefinition def = {
						.fromEnd = {.position = previous, .direction = dir},
						.toEnd = {.position = next, .direction = {}},
				};
				out += create(geoData, def);
				dir = out.back()->endAt(next)->dir;
			});
	return out;
}
