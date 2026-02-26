#version 460 core

layout(binding = 0) uniform sampler2D textureAlbedo;
layout(location = 0) out vec4 bNormal;
layout(location = 1) out vec4 bAlbedoSpec;

include(`materialDetail.glsl')
include(`materialCommon.glsl')
in vec2 gTexCoords;
in vec3 gNormal;
in vec4 gColour;
flat in MaterialDetail gMaterial;

void
main()
{
	vec4 textureColour = getTextureColour(gMaterial, gTexCoords);
	float opaque = step(0.5, mix(textureColour.a, 1, gColour.a));
	bNormal = vec4(gNormal, opaque);
	gl_FragDepth = mix(1.0, gl_FragCoord.z, opaque);
	bAlbedoSpec = mix(textureColour, vec4(gColour.rgb, 1), gColour.a);
}
