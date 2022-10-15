#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderFreeExtend : public EditNetwork::Builder {
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray & ray) override;
	void move(Network * network, const GeoData * geoData, const SDL_MouseMotionEvent & e, const Ray & ray) override;

	std::optional<glm::vec3> p1;
};
