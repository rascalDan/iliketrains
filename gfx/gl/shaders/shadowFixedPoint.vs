#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
const mat4 model = mat4(1);

include(`commonShadowPoint.glsl')
