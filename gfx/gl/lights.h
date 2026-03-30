#pragma once

#include "config/types.h"

struct LightCommonVertex {
	RelativePosition3D position;
	RGB colour;
	RelativeDistance kq;
};

struct SpotLightVertex : LightCommonVertex {
	Direction3D direction;
	Angle arc;
};

struct PointLightVertex : LightCommonVertex { };
