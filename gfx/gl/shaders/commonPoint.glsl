include(`getMaterialDetail.glsl')

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
