#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
const mat4 model = mat4(1);

include(`commonPoint.glsl')
