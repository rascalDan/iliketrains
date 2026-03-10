#version 460 core
#extension GL_ARB_shading_language_include : enable

layout(binding = 1) uniform usampler2DRect materialData;

#include "getMaterialDetail.glsl"
#include "materialDetail.glsl"
#include "meshIn.glsl"

out vec3 FragPos;
out vec2 TexCoords;
flat out MaterialDetail Material;
out vec3 Normal;
out vec4 Colour;

void
main()
{
	TexCoords = texCoord;
	Material = getMaterialDetail(material);
	FragPos = position;
	Colour = colour;
	Normal = normal;
}
