#ifndef COMMON_SHADOW_POINT_INCLUDED
#define COMMON_SHADOW_POINT_INCLUDED

out vec4 vworldPos;

void
main()
{
	vec3 worldPos = model * position;
	vworldPos = vec4(worldPos - viewPoint + modelPos, 1);
#ifdef TEXTURES
	TexCoords = texCoord;
	Material = getMaterialDetail(material);
#endif
}

#endif
