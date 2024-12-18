#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderJoin : public EditNetwork::Builder {
private:
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e,
			const Ray<GlobalPosition3D> & ray) override;
	void move(Network * network, const GeoData * geoData, const SDL_MouseMotionEvent & e,
			const Ray<GlobalPosition3D> & ray) override;

	Link::CCollection create(Network * network, const Node::Ptr & p1, const Node::Ptr & p2) const;

	Node::Ptr p1;
};
