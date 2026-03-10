#ifndef COMMON_POINT_INCLUDED
#define COMMON_POINT_INCLUDED

#include "getMaterialDetail.glsl"

void
main()
{
	FragPos = (model * position) + modelPos - viewPoint;
	TexCoords = texCoord;
	Normal = (model * normal);
	Colour = colour;
	Material = getMaterialDetail(material);

	gl_Position = viewProjection * vec4(FragPos, 1);
}

#endif
