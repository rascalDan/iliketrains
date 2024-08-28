#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in mat3 model;
layout(location = 3) in ivec3 worldPos;
uniform ivec3 viewPoint;
uniform vec3 centre;

out mat3 vmodel;
out ivec3 vworldPos;

void
main()
{
	vmodel = model;
	vworldPos = worldPos - viewPoint + ivec3(centre);
}
