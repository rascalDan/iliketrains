#version 330 core

include(`meshIn.glsl')

uniform mat4 viewProjection;

void
main()
{
	gl_Position = viewProjection * vec4(position, 1.0);
	gl_Position.z = max(gl_Position.z, -1);
}
