#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;

void
main()
{
	FragPos = position;
	TexCoords = texCoord;
	Normal = normal;
	Colour = colour;
	Material = material;

	gl_Position = viewProjection * vec4(position, 1.0);
}
