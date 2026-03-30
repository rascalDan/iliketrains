#version 460 core
#extension GL_ARB_shading_language_include : enable

layout(binding = 1) uniform usampler2DRect materialData;

#include "commonLocationData.glsl"
#include "materialInterface.glsl"
#include "meshIn.glsl"

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
layout(location = 5) in uint index;
mat3 model = mat3(locations[cldIndex[index]].rotationMatrix);
ivec3 modelPos = locations[cldIndex[index]].position.xyz;

#include "commonPoint.glsl"
