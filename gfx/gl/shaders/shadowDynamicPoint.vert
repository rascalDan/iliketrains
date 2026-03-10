#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "meshIn.glsl"

uniform ivec3 viewPoint;
uniform mat3 model;
uniform ivec3 modelPos;

#include "commonShadowPoint.glsl"
