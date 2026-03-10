#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "materialDetail.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 viewProjection[8];
uniform mat4 view[8];
in vec3 FragPos[];
in vec2 TexCoords[];
flat in MaterialDetail Material[];
in vec3 Normal[];
in vec4 Colour[];
out vec2 gTexCoords;
out vec3 gNormal;
out vec4 gColour;
flat out MaterialDetail gMaterial;

void
main()
{
	for (gl_Layer = 0; gl_Layer < viewProjection.length(); ++gl_Layer) {
		for (int v = 0; v < FragPos.length(); ++v) {
			gl_Position = viewProjection[gl_Layer] * vec4(FragPos[v], 1);
			gNormal = (view[gl_Layer] * vec4(Normal[v], 1)).xyz;
			gTexCoords = TexCoords[v];
			gMaterial = Material[v];
			gColour = Colour[v];
			EmitVertex();
		}
		EndPrimitive();
	}
}
