#version 330 core

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec4 Colour;

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
	gAlbedoSpec = mix(texture(texture0, TexCoords), vec4(Colour.rgb, 1), Colour.a);
}
