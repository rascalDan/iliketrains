#ifndef COMMON_SHADOW_POINT_GEOM_INCLUDED
#define COMMON_SHADOW_POINT_GEOM_INCLUDED

uniform mat4 viewProjection[4];
uniform int viewProjections;
in vec4 vworldPos[];
layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

#ifdef TEXTURES
#	include "materialDetail.glsl"

in vec2 TexCoords[];
out vec2 texCoord;
flat in MaterialDetail Material[];
flat out MaterialDetail material;
#endif

void
main()
{
	for (int vp = 0; vp < viewProjections; ++vp) {
		for (int v = 0; v < vworldPos.length(); ++v) {
			gl_Position = viewProjection[vp] * vworldPos[v];
			gl_Position.z = max(gl_Position.z, -1);
			gl_Layer = vp;
#ifdef TEXTURES
			texCoord = TexCoords[v];
			material = Material[v];
#endif
			EmitVertex();
		}
		EndPrimitive();
	}
}

#endif
