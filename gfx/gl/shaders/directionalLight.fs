#version 330 core
#extension GL_ARB_shading_language_420pack : enable

const int MAX_MAPS = 3;

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D shadowMap;

uniform vec3 lightDirection;
uniform vec3 lightColour;
uniform mat4 lightViewProjection[MAX_MAPS];
uniform vec4 shadowMapRegion[MAX_MAPS];
uniform uint lightViewProjectionCount;

const vec3 e1 = vec3(-1, -1, -1), e2 = vec3(1, 1, 1);

float
insideShadowCube(vec3 v)
{
	const vec3 s = step(e1, v) - step(e2, v);
	return s.x * s.y * s.z;
}

float
isShaded(vec3 Position)
{
	for (uint m = 0u; m < lightViewProjectionCount; m++) {
		vec3 PositionInLightSpace = (lightViewProjection[m] * vec4(Position, 1.0f)).xyz;
		const float inside = insideShadowCube(PositionInLightSpace);
		if (inside > 0) {
			const float lightSpaceDepth
					= texture(shadowMap, PositionInLightSpace.xy * shadowMapRegion[m].xy + shadowMapRegion[m].zw).r;
			return step(lightSpaceDepth, PositionInLightSpace.z * .5 + .5);
		}
	}
	return 0;
}

void
main()
{
	const vec3 Position = texture(gPosition, TexCoords).xyz;
	const vec3 Normal = texture(gNormal, TexCoords).rgb;
	const float shaded = isShaded(Position);
	FragColor = (1 - shaded) * max(dot(-lightDirection, Normal) * lightColour, 0);
}
