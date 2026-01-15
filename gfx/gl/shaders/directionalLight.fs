#version 330 core
#extension GL_ARB_shading_language_420pack : enable

const int MAX_MAPS = 4;

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform isampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2DArray shadowMap;

uniform vec3 lightDirection;
uniform vec3 lightColour;
uniform ivec3 lightPoint;
uniform mat4 lightViewProjection[MAX_MAPS];
uniform uint lightViewProjectionCount;

float
getShadow(vec3 positionInLightSpace, float m, vec2 texelSize)
{
	float shadow = 0.0;
	for (float x = -texelSize.x; x <= texelSize.x; x += texelSize.x) {
		for (float y = -texelSize.y; y <= texelSize.y; y += texelSize.y) {
			const float lightSpaceDepth = texture(shadowMap, vec3(positionInLightSpace.xy + vec2(x, y), m)).r;
			shadow += step(positionInLightSpace.z, lightSpaceDepth + 0.001);
		}
	}
	return shadow / 9.0;
}

float
insideShadowCube(vec3 v, vec2 texelSize)
{
	const vec3 s = step(vec3(texelSize, 0), v) - step(vec3(1 - texelSize, 1), v);
	return s.x * s.y * s.z;
}

float
isShaded(vec4 Position)
{
	const vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
	for (uint m = 0u; m < lightViewProjectionCount; m++) {
		const vec3 positionInLightSpace = (lightViewProjection[m] * Position).xyz;
		const float inside = insideShadowCube(positionInLightSpace, texelSize);
		if (inside > 0) {
			return getShadow(positionInLightSpace, m, texelSize);
		}
	}
	return 1.0;
}

void
main()
{
	const vec4 Position = vec4(texture(gPosition, TexCoords).xyz - lightPoint, 1);
	const vec3 Normal = texture(gNormal, TexCoords).rgb;
	const float shaded = isShaded(Position);
	FragColor = shaded * max(dot(-lightDirection, Normal) * lightColour, 0);
}
