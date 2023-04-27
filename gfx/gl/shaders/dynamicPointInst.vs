#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
layout(location = 5) in mat4 model;

include(`commonPoint.glsl')
