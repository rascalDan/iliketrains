#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "meshIn.glsl"

uniform ivec3 viewPoint;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

#include "commonShadowPoint.glsl"
