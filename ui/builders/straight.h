#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderStraight : public EditNetwork::Builder {
private:
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e,
			const Ray<GlobalPosition3D> & ray) override;
	void move(Network * network, const GeoData * geoData, const SDL_MouseMotionEvent & e,
			const Ray<GlobalPosition3D> & ray) override;

public:
	Link::CCollection create(Network * network, GlobalPosition3D p1, GlobalPosition3D p2) const;

private:
	std::optional<GlobalPosition3D> p1;
};
