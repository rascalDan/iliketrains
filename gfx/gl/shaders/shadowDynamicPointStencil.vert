#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "commonLocationData.glsl"

layout(location = 0) in uint index;
uniform ivec3 viewPoint;
uniform vec3 centre;

out float vmodelYaw;
out ivec3 vworldPos;

void
main()
{
	vmodelYaw = locations[cldIndex[index]].rotation.x;
	vworldPos = locations[cldIndex[index]].position.xyz - viewPoint + ivec3(centre);
}
