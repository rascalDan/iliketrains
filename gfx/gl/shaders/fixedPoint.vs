#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
const mat4 model = mat4(1);
const vec3 modelPos = vec3(0);

include(`commonPoint.glsl')
