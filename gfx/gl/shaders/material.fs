#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`materialInterface.glsl')
include(`materialOut.glsl')

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform usampler2DRect material;

float map(uint mapmode, float value)
{
	switch (mapmode) {
		case 0u: // Repeat
			return fract(value);
		case 1u: // Clamp to edge
			return clamp(0.0, 1.0, value);
		case 2u: // Mirror
			discard;
		case 3u: // Decal
			if (value != clamp(0.0, 1.0, value)) {
				discard;
			}
	}
	return 0;
}

vec2 map(uint mapmodeU, uint mapmodeV, vec2 value)
{
	return vec2(map(mapmodeU, value.x), map(mapmodeV, value.y));
}

vec4 getTextureColour(uint midx, vec2 uv)
{
	if (midx > 0u) {
		const vec2 tSize = textureSize(texture0, 0);
		const vec4 sPosSize = texture(material, uvec2(0, midx - 1u));
		const uvec4 sMode = texture(material, uvec2(1, midx - 1u));
		const uint mapmodeU = sMode.x & 0xFu;
		const uint mapmodeV = (sMode.x & 0xF0u) >> 1;
		uv = (sPosSize.xy + sPosSize.zw * map(mapmodeU, mapmodeV, uv)) / tSize;
	}
	return texture(texture0, uv);
}

void
main()
{
	vec4 textureColour = getTextureColour(Material, TexCoords);
	float opaque = step(0.5, mix(textureColour.a, 1, Colour.a));
	gPosition = vec4(FragPos, opaque);
	gNormal = vec4(Normal, opaque);
	gl_FragDepth = mix(1.0, gl_FragCoord.z, opaque);
	gAlbedoSpec = mix(textureColour, vec4(Colour.rgb, 1), Colour.a);
}
