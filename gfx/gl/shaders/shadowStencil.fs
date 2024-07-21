#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D textureAlbedo;

include(`materialInterface.glsl')
include(`materialCommon.glsl')

void
main()
{
	if (getTextureColour(Material, TexCoords).a < 0.5) {
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}
