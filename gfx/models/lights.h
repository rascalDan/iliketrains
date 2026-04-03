#pragma once

#include "config/types.h"

struct LightCommon {
	RelativePosition3D position;
	RGB colour;
	RelativeDistance kq;
};

struct LightCommonVertex {
	uint32_t parentObject;
};

struct SpotLightDef : LightCommon {
	Direction3D direction;
	Angle arc;
};

struct PointLightDef : LightCommon { };

struct SpotLightVertex : SpotLightDef, LightCommonVertex {
	SpotLightVertex(const SpotLightDef &, uint32_t);
};

struct PointLightVertex : PointLightDef, LightCommonVertex {
	PointLightVertex(const PointLightDef &, uint32_t);
};

