#version 460 core
#extension GL_ARB_shading_language_include : enable

layout(binding = 1) uniform usampler2DRect materialData;

#include "materialInterface.glsl"
#include "meshIn.glsl"

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
uniform mat3 model;
uniform ivec3 modelPos;

#include "commonPoint.glsl"
