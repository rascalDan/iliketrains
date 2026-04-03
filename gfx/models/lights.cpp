#include "lights.h"

SpotLightVertex::SpotLightVertex(const SpotLightDef & light, uint32_t parentObjectIdx) :
	SpotLightDef {light}, LightCommonVertex {parentObjectIdx}
{
}

PointLightVertex::PointLightVertex(const PointLightDef & light, uint32_t parentObjectIdx) :
	PointLightDef {light}, LightCommonVertex {parentObjectIdx}
{
}
