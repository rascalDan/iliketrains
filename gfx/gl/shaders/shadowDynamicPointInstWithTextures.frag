#version 460 core
#extension GL_ARB_shading_language_include : enable

layout(binding = 3) uniform sampler2D textureAlbedo;

#include "materialCommon.glsl"
#include "materialDetail.glsl"

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
