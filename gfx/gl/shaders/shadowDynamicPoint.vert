#version 460 core

include(`meshIn.glsl')

uniform ivec3 viewPoint;
uniform mat3 model;
uniform ivec3 modelPos;

include(`commonShadowPoint.glsl')
