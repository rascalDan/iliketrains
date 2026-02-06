#version 460 core

layout(binding = 3) uniform sampler2D textureAlbedo;

include(`materialInterface.glsl')
include(`materialCommon.glsl')

in vec2 texCoord;
flat in MaterialDetail material;

void
main()
{
	if (getTextureColour(material, texCoord).a < 0.5) {
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}
