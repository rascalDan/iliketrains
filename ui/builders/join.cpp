#include "join.h"
#include <game/geoData.h>

std::string
BuilderJoin::hint() const
{
	if (p1) {
		return "Pick second node";
	}
	return "Pick first node";
}

void
BuilderJoin::move(Network * network, const GeoData *, const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> & ray)
{
	if (p1) {
		if (const auto p = network->intersectRayNodes(ray)) {
			candidateLinks = network->candidateJoins(p1->pos, p->pos);
		}
		else {
			candidateLinks.clear();
		}
	}
}

void
BuilderJoin::click(
		Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (const auto p = network->intersectRayNodes(ray)) {
				if (p1) {
					create(network, geoData, p1, p);
					p1.reset();
					candidateLinks.clear();
				}
				else {
					p1 = p;
				}
			}
			return;
		case SDL_BUTTON_MIDDLE:
			p1.reset();
			return;
	}
}

Link::CCollection
BuilderJoin::create(Network * network, const GeoData * geoData, const Node::Ptr & p1, const Node::Ptr & p2) const
{
	const auto links = network->addJoins(geoData, p1->pos, p2->pos);
	setHeightsFor(network, links);
	return links;
}
