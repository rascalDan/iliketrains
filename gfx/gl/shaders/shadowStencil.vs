#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`meshIn.glsl')
uniform mat4 viewProjection;

void
main()
{
	gl_Position = viewProjection * vec4(position, 1);
}
