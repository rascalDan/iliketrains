#include "network.h"
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

template<typename T, typename... Links>
void
NetworkOf<T, Links...>::render(const SceneShader & shader) const
{
	if constexpr (std::is_base_of_v<Renderable, T>) {
		shader.absolute.use();
		texture->bind();
		links.apply(&Renderable::render, shader);
	}
}

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
	if (glm::length(RelativePosition3D(start - end)) < 2000.F) {
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
NetworkOf<T, Links...>::addStraight(GlobalPosition3D n1, GlobalPosition3D n2)
{
	return {addLink<typename T::StraightLink>(n1, n2)};
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addJoins(GlobalPosition3D start, GlobalPosition3D end)
{
	if (glm::length(RelativePosition3D(start - end)) < 2000.F) {
		return {};
	}
	const auto defs = genCurveDef(start, end, findNodeDirection(nodeAt(start)), findNodeDirection(nodeAt(end)));
	const auto & [c1s, c1e, c1c] = defs.first;
	const auto & [c2s, c2e, c2c] = defs.second;
	return {addLink<typename T::CurveLink>(c1s, c1e, c1c), addLink<typename T::CurveLink>(c2s, c2e, c2c)};
}

template<typename T, typename... Links>
Link::CCollection
NetworkOf<T, Links...>::addExtend(GlobalPosition3D start, GlobalPosition3D end)
{
	const auto [cstart, cend, centre] = genCurveDef(start, end, findNodeDirection(nodeAt(start)));
	return {addLink<typename T::CurveLink>(cstart, cend, centre)};
}
