#include "straight.h"
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
BuilderStraight::move(Network * network, const GeoData * geoData, const SDL_MouseMotionEvent &, const Ray & ray)
{
	if (p1) {
		if (const auto p = geoData->intersectRay(ray)) {
			candidateLinks.objects = network->candidateStraight(*p1, *p);
		}
		else {
			candidateLinks.removeAll();
		}
	}
}

void
BuilderStraight::click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray & ray)
{
	switch (e.button) {
		case SDL_BUTTON_LEFT:
			if (const auto p = geoData->intersectRay(ray)) {
				if (p1) {
					create(network, *p1, *p);
					candidateLinks.removeAll();
				}
				p1 = *p;
			}
			return;
		case SDL_BUTTON_MIDDLE:
			p1.reset();
			candidateLinks.removeAll();
			return;
	}
}

void
BuilderStraight::create(Network * network, glm::vec3 p1, glm::vec3 p2) const
{
	network->addStraight(p1, p2);
}
