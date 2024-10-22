#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform usampler2DRect materialData;

include(`meshIn.glsl')
include(`materialDetail.glsl')
include(`getMaterialDetail.glsl')

out vec3 FragPos;
out vec2 TexCoords;
flat out MaterialDetail Material;

void
main()
{
	TexCoords = texCoord;
	Material = getMaterialDetail(material);
	FragPos = position;
}
