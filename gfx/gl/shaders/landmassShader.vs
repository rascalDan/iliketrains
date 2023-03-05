#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec4 colour;

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
