float
map(uint mapmode, float value)
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

vec2
map(uvec2 mapmode, vec2 value)
{
	return vec2(map(mapmode.x, value.x), map(mapmode.y, value.y));
}

vec4
getTextureColour(MaterialDetail mat, vec2 uv)
{
	if (mat.textureSize.x > 0) {
		const vec2 tSize = textureSize(textureAlbedo, 0);
		uv = (mat.textureOrigin + mat.textureSize * map(mat.mapmode, uv)) / tSize;
	}
	return texture(textureAlbedo, uv);
}
