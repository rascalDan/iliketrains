#version 460 core

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 2) uniform sampler2D gAlbedoSpec;
layout(binding = 3) uniform sampler2D gIllumination;

void
main()
{
	const vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
	const vec3 Illumination = texture(gIllumination, TexCoords).rgb;

	FragColor = Albedo * Illumination;
}
