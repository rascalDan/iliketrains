#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
layout(location = 5) in mat4 model;

include(`commonShadowPoint.glsl')
