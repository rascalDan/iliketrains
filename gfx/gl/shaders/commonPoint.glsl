layout(binding = 1) uniform usampler2DRect materialData;

MaterialDetail
getMaterialDetail(uint midx)
{
	if (midx > 0u) {
		const vec4 sPosSize = texture(materialData, uvec2(0, midx - 1u));
		const uvec4 sMode = texture(materialData, uvec2(1, midx - 1u));
		const uint mapmodeU = sMode.x & 0xFu;
		const uint mapmodeV = (sMode.x & 0xF0u) >> 1;
		return MaterialDetail(sPosSize.xy, sPosSize.zw, uvec2(mapmodeU, mapmodeV));
	}
	return MaterialDetail(vec2(0, 0), vec2(0, 0), uvec2(0, 0));
}

void
main()
{
	FragPos = (model * position) + modelPos;
	TexCoords = texCoord;
	Normal = (model * normal);
	Colour = colour;
	Material = getMaterialDetail(material);

	gl_Position = viewProjection * vec4(FragPos - viewPoint, 1);
}
