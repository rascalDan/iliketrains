#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
uniform mat3 model;
uniform ivec3 modelPos;

include(`commonShadowPoint.glsl')
