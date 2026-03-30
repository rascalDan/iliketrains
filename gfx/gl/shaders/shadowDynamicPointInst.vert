#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "commonLocationData.glsl"
#include "meshIn.glsl"

uniform ivec3 viewPoint;
layout(location = 5) in uint index;
mat3 model = mat3(locations[cldIndex[index]].rotationMatrix);
ivec3 modelPos = locations[cldIndex[index]].position.xyz;

#include "commonShadowPoint.glsl"
