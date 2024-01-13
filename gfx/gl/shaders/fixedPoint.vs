#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
const mat3 model = mat3(1);
const vec3 modelPos = ivec3(0);

include(`commonPoint.glsl')
