#include "network.h"
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

template<typename T>
void
NetworkOf<T>::render(const SceneShader & shader) const
{
	if constexpr (std::is_base_of_v<Renderable, T>) {
		shader.absolute.use();
		texture->bind();
		links.apply(&Renderable::render, shader);
	}
}

template<typename T>
void
NetworkOf<T>::joinLinks(const Link::Ptr & l) const
{
	for (const auto & ol : links.objects) {
		Network::joinLinks(l, ol);
	}
}

template<typename T>
Link::Ptr
NetworkOf<T>::intersectRayLinks(const Ray & ray) const
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

template<typename T>
float
NetworkOf<T>::findNodeDirection(Node::AnyCPtr n) const
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

template<typename T>
Link::CCollection
NetworkOf<T>::candidateStraight(glm::vec3 n1, glm::vec3 n2)
{
	return {candidateLink<typename T::StraightLink>(n1, n2)};
}

template<typename T>
Link::CCollection
NetworkOf<T>::candidateJoins(glm::vec3 start, glm::vec3 end)
{
	if (glm::distance(start, end) < 2.F) {
		return {};
	}
	const auto defs = genCurveDef(
			start, end, findNodeDirection(candidateNodeAt(start).first), findNodeDirection(candidateNodeAt(end).first));
	const auto & [c1s, c1e, c1c] = defs.first;
	const auto & [c2s, c2e, c2c] = defs.second;
	return {candidateLink<typename T::CurveLink>(c1s, c1e, c1c), candidateLink<typename T::CurveLink>(c2s, c2e, c2c)};
}

template<typename T>
Link::CCollection
NetworkOf<T>::candidateExtend(glm::vec3 start, glm::vec3 end)
{
	const auto [cstart, cend, centre] = genCurveDef(start, end, findNodeDirection(candidateNodeAt(start).first));
	return {candidateLink<typename T::CurveLink>(cstart, cend, centre)};
}

template<typename T>
Link::CCollection
NetworkOf<T>::addStraight(glm::vec3 n1, glm::vec3 n2)
{
	return {addLink<typename T::StraightLink>(n1, n2)};
}

template<typename T>
Link::CCollection
NetworkOf<T>::addJoins(glm::vec3 start, glm::vec3 end)
{
	if (glm::distance(start, end) < 2.F) {
		return {};
	}
	const auto defs = genCurveDef(start, end, findNodeDirection(nodeAt(start)), findNodeDirection(nodeAt(end)));
	const auto & [c1s, c1e, c1c] = defs.first;
	const auto & [c2s, c2e, c2c] = defs.second;
	return {addLink<typename T::CurveLink>(c1s, c1e, c1c), addLink<typename T::CurveLink>(c2s, c2e, c2c)};
}

template<typename T>
Link::CCollection
NetworkOf<T>::addExtend(glm::vec3 start, glm::vec3 end)
{
	const auto [cstart, cend, centre] = genCurveDef(start, end, findNodeDirection(nodeAt(start)));
	return {addLink<typename T::CurveLink>(cstart, cend, centre)};
}
