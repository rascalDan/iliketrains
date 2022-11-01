#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 viewProjection;

void
main()
{
	FragPos = position;
	TexCoords = texCoord;
	Normal = normal;

	gl_Position = viewProjection * vec4(position, 1.0);
}
