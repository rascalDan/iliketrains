struct MaterialDetail {
	vec2 textureOrigin;
	vec2 textureSize;
	uvec2 mapmode;
};

ifelse(TYPE, .fs, in, out) vec3 FragPos;
ifelse(TYPE, .fs, in, out) vec2 TexCoords;
ifelse(TYPE, .fs, in, out) vec3 Normal;
ifelse(TYPE, .fs, in, out) vec4 Colour;
flat
ifelse(TYPE, .fs, in, out)
MaterialDetail Material;
