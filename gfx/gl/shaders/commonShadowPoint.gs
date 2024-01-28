#version 330 core
#extension GL_ARB_viewport_array : enable

uniform mat4 viewProjection[4];
uniform int viewProjections;
in vec4 vworldPos[];
layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

void
main()
{
	for (int vp = 0; vp < viewProjections; ++vp) {
		for (int v = 0; v < vworldPos.length(); ++v) {
			gl_Position = viewProjection[vp] * vworldPos[v];
			gl_Position.z = max(gl_Position.z, -1);
			gl_Layer = vp;
			EmitVertex();
		}
		EndPrimitive();
	}
}
