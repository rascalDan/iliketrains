#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;
uniform mat4 model;

void
main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	gl_Position.z = max(gl_Position.z, -1);
}
