#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 viewProjection;
uniform mat4 model;

void
main()
{
	vec4 worldPos = model * vec4(position, 1.0);

	FragPos = worldPos.xyz;
	TexCoords = texCoord;
	Normal = (model * vec4(normal, 0.0)).xyz;

	gl_Position = viewProjection * worldPos;
}
