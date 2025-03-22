#include "freeExtend.h"
#include <game/geoData.h>

std::string
BuilderFreeExtend::hint() const
{
	if (p1) {
		return "Pick next point";
	}
	return "Pick start node";
}

void
BuilderFreeExtend::move(
		Network * network, const GeoData * geoData, const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> & ray)
{
	if (p1) {
		if (const auto p = network->intersectRayNodes(ray)) {
			candidateLinks = network->candidateJoins(*p1, p->pos);
		}
		else if (const auto p = geoData->intersectRay(ray)) {
			candidateLinks = network->candidateExtend(*p1, p->first);
		}
		else {
			candidateLinks.removeAll();
		}
	}
	else {
		candidateLinks.removeAll();
	}
}

void
BuilderFreeExtend::click(
		Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (p1) {
				if (const auto p = network->intersectRayNodes(ray)) {
					createJoin(network, geoData, *p1, p->pos);
					p1 = p->pos;
				}
				else if (const auto p = geoData->intersectRay(ray)) {
					createExtend(network, geoData, *p1, p->first);
					p1 = p->first;
				}
			}
			else {
				if (const auto p = network->intersectRayNodes(ray)) {
					p1 = p->pos;
				}
			}
			return;
		case SDL_BUTTON_MIDDLE:
			p1.reset();
			return;
	}
}

Link::CCollection
BuilderFreeExtend::createJoin(
		Network * network, const GeoData * geoData, GlobalPosition3D p1, GlobalPosition3D p2) const
{
	const auto links = network->addJoins(geoData, p1, p2);
	setHeightsFor(network, links);
	return links;
}

Link::CCollection
BuilderFreeExtend::createExtend(
		Network * network, const GeoData * geoData, GlobalPosition3D p1, GlobalPosition3D p2) const
{
	const auto links = network->addExtend(geoData, p1, p2);
	setHeightsFor(network, links);
	return links;
}
