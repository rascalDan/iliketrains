#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform mat4 model;

include(`commonShadowPoint.glsl')
