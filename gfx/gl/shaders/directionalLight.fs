#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 1) uniform sampler2D gNormal;

uniform vec3 lightDirection;
uniform vec3 lightColour;

void
main()
{
	const vec3 Normal = texture(gNormal, TexCoords).rgb;
	FragColor = dot(-lightDirection, Normal) * lightColour;
}
