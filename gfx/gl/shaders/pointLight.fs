#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
uniform ivec4 viewPort;
uniform vec3 colour;
uniform float kq;
in vec3 geo_centre;

void
main()
{
	const vec2 texCoord = gl_FragCoord.xy / viewPort.zw;
	const vec3 position = texture(gPosition, texCoord).xyz;
	const vec3 normal = texture(gNormal, texCoord).xyz;
	const vec3 lightv = position - geo_centre;
	const float lightDist = length(lightv);
	const vec3 lightDirection = normalize(lightv);
	const float normalDot = dot(-lightDirection, normal);
	if (normalDot < 0) {
		discard;
	}
	FragColor = (colour * normalDot) / (1 + (kq * pow(lightDist, 2)));
}
