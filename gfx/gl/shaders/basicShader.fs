#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`materialInterface.glsl')
include(`geometryOut.glsl')

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform usampler2DRect material;

vec4 getTextureColour(uint midx, vec2 uv)
{
	if (midx > 0u) {
		const vec2 tSize = textureSize(texture0, 0);
		const vec4 sPosSize = texture(material, uvec2(0, midx - 1u));
		uv = (sPosSize.xy + sPosSize.zw * fract(uv)) / tSize;
	}
	return texture(texture0, uv);
}

void
main()
{
	vec4 textureColour = getTextureColour(Material, TexCoords);
	float clear = round(mix(textureColour.a, 1, Colour.a));
	gPosition = vec4(FragPos, clear);
	gNormal = vec4(Normal, clear);
	gAlbedoSpec = mix(textureColour, vec4(Colour.rgb, 1), Colour.a);
}
