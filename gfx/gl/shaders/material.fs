#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`materialInterface.glsl')
include(`materialOut.glsl')

layout(binding = 0) uniform sampler2D texture0;

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

vec2 map(uvec2 mapmode, vec2 value)
{
	return vec2(map(mapmode.x, value.x), map(mapmode.y, value.y));
}

vec4 getTextureColour(MaterialDetail mat, vec2 uv)
{
	if (mat.textureSize.x > 0) {
		const vec2 tSize = textureSize(texture0, 0);
		uv = (mat.textureOrigin + mat.textureSize * map(mat.mapmode, uv)) / tSize;
	}
	return texture(texture0, uv);
}

void
main()
{
	vec4 textureColour = getTextureColour(Material, TexCoords);
	float opaque = step(0.5, mix(textureColour.a, 1, Colour.a));
	gPosition = ivec4(FragPos, opaque);
	gNormal = vec4(Normal, opaque);
	gl_FragDepth = mix(1.0, gl_FragCoord.z, opaque);
	gAlbedoSpec = mix(textureColour, vec4(Colour.rgb, 1), Colour.a);
}
