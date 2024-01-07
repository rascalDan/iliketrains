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
			candidateLinks.objects = network->candidateJoins(p1->pos, p->pos);
		}
		else {
			candidateLinks.removeAll();
		}
	}
}

void
BuilderJoin::click(
		Network * network, const GeoData *, const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (const auto p = network->intersectRayNodes(ray)) {
				if (p1) {
					create(network, p1, p);
					p1.reset();
					candidateLinks.removeAll();
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

void
BuilderJoin::create(Network * network, const Node::Ptr & p1, const Node::Ptr & p2) const
{
	network->addJoins(p1->pos, p2->pos);
}
