#version 330 core
#extension GL_ARB_viewport_array : enable

ifdef(`TEXTURES', include(`materialDetail.glsl'))

uniform mat4 viewProjection[4];
uniform int viewProjections;
in vec4 vworldPos[];
layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

ifdef(`TEXTURES', in vec2 TexCoords[]; out vec2 texCoord;)
ifdef(`TEXTURES', flat in MaterialDetail Material[]; flat out MaterialDetail material;)

void
main()
{
	for (int vp = 0; vp < viewProjections; ++vp) {
		for (int v = 0; v < vworldPos.length(); ++v) {
			gl_Position = viewProjection[vp] * vworldPos[v];
			gl_Position.z = max(gl_Position.z, -1);
			gl_Layer = vp;
			ifdef(`TEXTURES', texCoord = TexCoords[v];)
			ifdef(`TEXTURES', material = Material[v];)
			EmitVertex();
		}
		EndPrimitive();
	}
}
