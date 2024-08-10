#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 4) uniform usampler2DRect materialData;

define(`TEXTURES', 1)
include(`materialInterface.glsl')
include(`getMaterialDetail.glsl')
include(`meshIn.glsl')

uniform ivec3 viewPoint;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

include(`commonShadowPoint.glsl')
