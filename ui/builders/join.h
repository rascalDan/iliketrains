#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderJoin : public EditNetwork::Builder {
	void render(const Shader &) const override;
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e, const Ray & ray) override;

	void create(Network * network, const Node::Ptr & p1, const Node::Ptr & p2) const;

	Node::Ptr p1;
};
