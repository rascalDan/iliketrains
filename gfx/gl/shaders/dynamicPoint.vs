#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
uniform mat4 model;
uniform vec3 modelPos;

include(`commonPoint.glsl')
