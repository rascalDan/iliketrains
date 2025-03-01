#include "collections.h"
#include "network.h"
#include <game/geoData.h>
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

template<typename T, typename... Links>
void
NetworkOf<T, Links...>::joinLinks(const Link::Ptr & l) const
{
	for (const auto & ol : links.objects) {
		Network::joinLinks(l, ol);
	}
}

template<typename T, typename... Links>
Link::Ptr
NetworkOf<T, Links...>::intersectRayLinks(const Ray<GlobalPosition3D> & ray) const
{
	// Click link
	if (const auto link = std::find_if(links.objects.begin(), links.objects.end(),
				[&ray](const std::shared_ptr<T> & link) {
					return link->intersectRay(ray);
				});
			link != links.objects.end()) {
		return *link;
	}
	return {};
}

template<typename T, typename... Links>
float
NetworkOf<T, Links...>::findNodeDirection(Node::AnyCPtr n) const
{
	for (const auto & l : links.objects) {
		for (const auto & e : l->ends) {
			// cppcheck-suppress useStlAlgorithm
			if (e.node.get() == n.get()) {
				return e.dir;
			}
		}
	}
	throw std::runtime_error("Node exists but couldn't find it");
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::candidateStraight(GlobalPosition3D n1, GlobalPosition3D n2)
{
	return {candidateLink<typename T::StraightLink>(n1, n2)};
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::candidateJoins(GlobalPosition3D start, GlobalPosition3D end)
{
	if (::distance(start, end) < 2000.F) {
		return {};
	}
	const auto defs = genCurveDef(
			start, end, findNodeDirection(candidateNodeAt(start).first), findNodeDirection(candidateNodeAt(end).first));
	const auto & [c1s, c1e, c1c] = defs.first;
	const auto & [c2s, c2e, c2c] = defs.second;
	return {candidateLink<typename T::CurveLink>(c1s, c1e, c1c), candidateLink<typename T::CurveLink>(c2s, c2e, c2c)};
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::candidateExtend(GlobalPosition3D start, GlobalPosition3D end)
{
	const auto [cstart, cend, centre] = genCurveDef(start, end, findNodeDirection(candidateNodeAt(start).first));
	return {candidateLink<typename T::CurveLink>(cstart, cend, centre)};
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addStraight(const GeoData * geoData, GlobalPosition3D n1, GlobalPosition3D n2)
{
	Link::CCollection out;
	geoData->walk(n1.xy(), n2, [geoData, &out, this, &n1](const GeoData::WalkStep & step) {
		if (step.previous.is_valid() && geoData->getSurface(step.current) != geoData->getSurface(step.previous)) {
			const auto surfaceEdgePosition = geoData->positionAt(GeoData::PointFace(step.exitPosition, step.current));
			out.emplace_back(addLink<typename T::StraightLink>(n1, surfaceEdgePosition));
			n1 = surfaceEdgePosition;
		}
	});
	out.emplace_back(addLink<typename T::StraightLink>(n1, n2));
	return out;
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addCurve(const GeoData * geoData, const GenCurveDef & curve)
{
	auto [cstart, cend, centre] = curve;
	Link::CCollection out;
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
	mergeClose(points, ::distance<3, GlobalDistance>, ::midpoint<3, GlobalDistance>, 2'000.F);
	std::ranges::transform(points | std::views::pairwise, std::back_inserter(out), [this, centre](const auto pair) {
		const auto [a, b] = pair;
		return this->addLink<typename T::CurveLink>(a, b, centre);
	});
	return out;
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addJoins(const GeoData * geoData, GlobalPosition3D start, GlobalPosition3D end)
{
	if (::distance(start, end) < 2000.F) {
		return {};
	}
	const auto defs = genCurveDef(start, end, findNodeDirection(nodeAt(start)), findNodeDirection(nodeAt(end)));
	return addCurve(geoData, defs.first) + addCurve(geoData, defs.second);
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addExtend(const GeoData * geoData, GlobalPosition3D start, GlobalPosition3D end)
{
	return addCurve(geoData, genCurveDef(start, end, findNodeDirection(nodeAt(start))));
}
