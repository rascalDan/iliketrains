#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

layout(binding = 0) uniform isampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
uniform ivec4 viewPort;
flat in vec4 geo_centre;
flat in vec3 geo_colour;
flat in float geo_kq;

void
main()
{
	const vec2 texCoord = gl_FragCoord.xy / viewPort.zw;
	const vec3 position = texture(gPosition, texCoord).xyz;
	const vec3 lightv = position - geo_centre.xyz;
	const float lightDist = length(lightv);
	if (lightDist > geo_centre.w) {
		discard;
	}
	const vec3 normal = texture(gNormal, texCoord).xyz;
	const vec3 lightDirection = normalize(lightv);
	const float normalDot = dot(-lightDirection, normal);
	if (normalDot < 0) {
		discard;
	}
	FragColor = (geo_colour * normalDot) / (1 + (geo_kq * pow(lightDist / 1000.0, 2)));
}
