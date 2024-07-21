#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform usampler2DRect materialData;

include(`meshIn.glsl')
include(`materialInterface.glsl')
include(`getMaterialDetail.glsl')
uniform mat4 viewProjection;

void
main()
{
	TexCoords = texCoord;
	Material = getMaterialDetail(material);

	gl_Position = viewProjection * vec4(position, 1);
}
