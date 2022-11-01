#version 330 core

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

out vec4 gPosition;
out vec4 gNormal;
out vec4 gAlbedoSpec;

uniform sampler2D texture0;

void
main()
{
	float clear = round(texture(texture0, TexCoords).a);
	gPosition = vec4(FragPos, clear);
	gNormal = vec4(Normal, clear);
	gAlbedoSpec = texture(texture0, TexCoords);
}
