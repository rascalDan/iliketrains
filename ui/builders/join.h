#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderJoin : public EditNetwork::Builder {
	void render(const Shader &) const override;
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray & ray) override;

	void create(Network * network, const NodePtr & p1, const NodePtr & p2) const;

	NodePtr p1;
};
