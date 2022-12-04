#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D shadowMap;

uniform vec3 lightDirection;
uniform vec3 lightColour;
uniform mat4 lightViewProjection;

void
main()
{
	const vec3 Position = texture(gPosition, TexCoords).xyz;
	const vec4 PositionInLightSpace = (lightViewProjection * vec4(Position, 1)) * 0.5 + 0.5;
	const float lightSpaceDepth = texture(shadowMap, PositionInLightSpace.xy).r;
	if (lightSpaceDepth < PositionInLightSpace.z) {
		discard;
	}
	const vec3 Normal = texture(gNormal, TexCoords).rgb;
	FragColor = max(dot(-lightDirection, Normal) * lightColour, 0);
}
