#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
const mat4 model = mat4(1);
const vec3 modelPos = vec3(0);

include(`commonShadowPoint.glsl')
