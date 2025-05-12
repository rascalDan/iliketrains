#include "network.h"
#include <game/geoData.h>
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

template<typename T, typename... Links>
void
NetworkOf<T, Links...>::joinLinks(const Link::Ptr & link) const
{
	for (const auto & oldLink : links) {
		Network::joinLinks(link, oldLink);
	}
}

template<typename T, typename... Links>
Link::Ptr
NetworkOf<T, Links...>::intersectRayLinks(const Ray<GlobalPosition3D> & ray) const
{
	// Click link
	if (const auto link = std::find_if(links.begin(), links.end(),
				[&ray](const std::shared_ptr<T> & link) {
					return link->intersectRay(ray);
				});
			link != links.end()) {
		return *link;
	}
	return {};
}

template<typename T, typename... Links>
float
NetworkOf<T, Links...>::findNodeDirection(Node::AnyCPtr n) const
{
	for (const auto & link : links) {
		for (const auto & end : link->ends) {
			// cppcheck-suppress useStlAlgorithm
			if (end.node.get() == n.get()) {
				return end.dir;
			}
		}
	}
	throw std::runtime_error("Node exists but couldn't find it");
}

template<typename T, typename... Links>
Link::Ptr
NetworkOf<T, Links...>::create(const GenStraightDef & def)
{
	return std::make_shared<typename T::StraightLink>(
			*this, candidateNodeAt(std::get<0>(def)).first, candidateNodeAt(std::get<1>(def)).first);
}

template<typename T, typename... Links>
Link::Ptr
NetworkOf<T, Links...>::create(const GenCurveDef & def)
{
	return std::make_shared<typename T::CurveLink>(
			*this, candidateNodeAt(std::get<0>(def)).first, candidateNodeAt(std::get<1>(def)).first, std::get<2>(def));
}

template<typename T, typename... Links>
bool
NetworkOf<T, Links...>::anyLinks() const
{
	return !(static_cast<const NetworkLinkHolder<Links> *>(this)->vertices.empty() && ...);
}

template<typename T, typename... Links>
void
NetworkOf<T, Links...>::add(GeoData * geoData, const Link::Ptr & link)
{
	const auto addIf = [this](auto && lptr) {
		if (lptr) {
			links.emplace(lptr);
			return true;
		}
		return false;
	};
	for (auto & end : link->ends) {
		end.node = nodeAt(end.node->pos);
	}
	if (!(addIf(std::dynamic_pointer_cast<Links>(link)) || ...)) {
		throw std::logic_error("Unsupported link type for network");
	}
	joinLinks(link);
	if (geoData) {
		geoData->setHeights(link->getBase(getBaseWidth()), GeoData::SetHeightsOpts {.surface = getBaseSurface()});
	}
}
