#include "join.h"
#include <game/geoData.h>

void
BuilderJoin::render(const Shader &) const
{
}

std::string
BuilderJoin::hint() const
{
	if (p1) {
		return "Pick second node";
	}
	return "Pick first node";
}

void
BuilderJoin::click(Network * network, const GeoData *, const SDL_MouseButtonEvent & e, const Ray & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (const auto p = network->intersectRayNodes(ray)) {
				if (p1) {
					create(network, p1, p);
					p1.reset();
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
BuilderJoin::create(Network * network, const NodePtr & p1, const NodePtr & p2) const
{
	network->addJoins(p1->pos, p2->pos);
}
