#version 330 core

include(`meshIn.glsl')
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec4 Colour;

uniform mat4 viewProjection;

void
main()
{
	FragPos = position;
	TexCoords = texCoord;
	Normal = normal;
	Colour = colour;

	gl_Position = viewProjection * vec4(position, 1.0);
}
