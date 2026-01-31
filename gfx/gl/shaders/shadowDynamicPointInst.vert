#version 460 core

include(`meshIn.glsl')

uniform ivec3 viewPoint;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

include(`commonShadowPoint.glsl')
