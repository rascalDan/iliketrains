#version 460 core
#extension GL_ARB_shading_language_include : enable
#define TEXTURES

layout(binding = 4) uniform usampler2DRect materialData;

#include "getMaterialDetail.glsl"
#include "materialInterface.glsl"
#include "meshIn.glsl"

uniform ivec3 viewPoint;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

#include "commonShadowPoint.glsl"
