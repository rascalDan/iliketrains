#version 330 core
#extension GL_ARB_viewport_array : enable

include(`materialDetail.glsl')

layout(triangles) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 viewProjection;
in vec3 FragPos[];
in vec2 TexCoords[];
flat in MaterialDetail Material[];
out vec2 gTexCoords;
flat out MaterialDetail gMaterial;

void
main()
{
	for (gl_Layer = 0; gl_Layer < 8; ++gl_Layer) {
		for (int v = 0; v < FragPos.length(); ++v) {
			gl_Position = viewProjection * vec4(FragPos[v], 1);
			gTexCoords = TexCoords[v];
			gMaterial = Material[v];
			EmitVertex();
		}
		EndPrimitive();
	}
}
