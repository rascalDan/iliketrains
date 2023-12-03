#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
const mat4 model = mat4(1);
const ivec3 modelPos = ivec3(0);

include(`commonShadowPoint.glsl')
