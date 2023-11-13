#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
layout(location = 5) in mat4 model;
layout(location = 9) in vec3 modelPos;

include(`commonPoint.glsl')
