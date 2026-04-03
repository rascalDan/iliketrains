#version 460 core
#extension GL_ARB_shading_language_include : enable
#define TEXTURES

layout(binding = 4) uniform usampler2DRect materialData;

#include "commonLocationData.glsl"
#include "getMaterialDetail.glsl"
#include "materialInterface.glsl"
#include "meshIn.glsl"

uniform ivec3 viewPoint;
layout(location = 5) in uint index;
mat3 model = mat3(locations[cldIndex[index]].rotationMatrix);
ivec3 modelPos = locations[cldIndex[index]].position.xyz;

#include "commonShadowPoint.glsl"
