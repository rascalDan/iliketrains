#version 330 core

include(`meshIn.glsl')

uniform ivec3 viewPoint;
const mat3 model = mat3(1);
const ivec3 modelPos = ivec3(0);

include(`commonShadowPoint.glsl')
