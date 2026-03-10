#version 460 core
#extension GL_ARB_shading_language_include : enable

layout(binding = 1) uniform usampler2DRect materialData;

#include "materialInterface.glsl"
#include "meshIn.glsl"

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

#include "commonPoint.glsl"
