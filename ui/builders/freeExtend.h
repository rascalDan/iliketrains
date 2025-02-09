#pragma once
#include "../editNetwork.h"

class Network;
class GeoData;

class BuilderFreeExtend : public EditNetwork::Builder {
private:
	std::string hint() const override;
	void click(Network * network, const GeoData * geoData, const SDL_MouseButtonEvent & e,
			const Ray<GlobalPosition3D> & ray) override;
	void move(Network * network, const GeoData * geoData, const SDL_MouseMotionEvent & e,
			const Ray<GlobalPosition3D> & ray) override;

public:
	Link::CCollection createJoin(Network * network, const GeoData *, GlobalPosition3D, GlobalPosition3D) const;
	Link::CCollection createExtend(Network * network, const GeoData *, GlobalPosition3D, GlobalPosition3D) const;

private:
	std::optional<GlobalPosition3D> p1;
};
