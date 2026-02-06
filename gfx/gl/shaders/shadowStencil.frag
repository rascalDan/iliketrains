#version 460 core

layout(binding = 0) uniform sampler2D textureAlbedo;

include(`materialDetail.glsl')
include(`materialCommon.glsl')
in vec2 gTexCoords;
flat in MaterialDetail gMaterial;

void
main()
{
	if (getTextureColour(gMaterial, gTexCoords).a < 0.5) {
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}
