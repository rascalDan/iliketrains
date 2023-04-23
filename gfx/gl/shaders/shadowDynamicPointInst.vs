#version 330 core

include(`meshIn.glsl')
layout(location = 5) in mat4 model;

uniform mat4 viewProjection;

void
main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	gl_Position.z = max(gl_Position.z, -1);
}
