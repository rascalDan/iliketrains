#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform mat4 model;

include(`commonPoint.glsl')
