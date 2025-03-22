#include "straight.h"
#include "stream_support.h"
#include <game/geoData.h>

std::string
BuilderStraight::hint() const
{
	if (p1) {
		return "Pick straight end point";
	}
	return "Pick straight start point";
}

void
BuilderStraight::move(
		Network * network, const GeoData * geoData, const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> & ray)
{
	if (p1) {
		if (const auto p = geoData->intersectRay(ray)) {
			candidateLinks = network->candidateStraight(*p1, p->first);
		}
		else {
			candidateLinks.removeAll();
		}
	}
}

void
BuilderStraight::click(
		Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (const auto p = geoData->intersectRay(ray)) {
				if (p1) {
					create(network, geoData, *p1, p->first);
					candidateLinks.removeAll();
					p1.reset();
				}
				else {
					p1 = p->first;
				}
			}
			return;
		case SDL_BUTTON_MIDDLE:
			p1.reset();
			candidateLinks.removeAll();
			return;
	}
}

Link::CCollection
BuilderStraight::create(Network * network, const GeoData * geoData, GlobalPosition3D p1, GlobalPosition3D p2) const
{
	const auto links = network->addStraight(geoData, p1, p2);
	setHeightsFor(network, links);
	return links;
}
