#version 460 core

layout(binding = 1) uniform usampler2DRect materialData;

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
const mat3 model = mat3(1);
const vec3 modelPos = ivec3(0);

include(`commonPoint.glsl')
