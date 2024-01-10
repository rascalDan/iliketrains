#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

layout(binding = 0) uniform isampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
uniform ivec4 viewPort;
// uniform vec3 colour;
// uniform float kq;
const vec3 colour = vec3(1);
const float kq = 0.01;
in vec4 geo_centre;
in vec4 geo_direction;

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
	const vec3 lightDirection = normalize(lightv);
	if (dot(lightDirection, geo_direction.xyz) < geo_direction.w) {
		discard;
	}
	const vec3 normal = texture(gNormal, texCoord).xyz;
	const float normalDot = dot(-lightDirection, normal);
	if (normalDot < 0) {
		discard;
	}
	FragColor = (colour * normalDot) / (1 + (kq * pow(lightDist / 1000.0, 2)));
}
