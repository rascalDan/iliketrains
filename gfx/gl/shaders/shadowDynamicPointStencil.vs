#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in ivec3 worldPos;
layout(location = 1) in float modelYaw;
uniform ivec3 viewPoint;
uniform vec3 centre;

out float vmodelYaw;
out ivec3 vworldPos;

void
main()
{
	vmodelYaw = modelYaw;
	vworldPos = worldPos - viewPoint + ivec3(centre);
}
