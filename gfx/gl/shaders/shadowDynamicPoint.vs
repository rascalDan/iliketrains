#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
uniform mat4 model;
uniform vec3 modelPos;

include(`commonShadowPoint.glsl')
