#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "materialOut.glsl"

in vec4 FragPos;
in vec2 TexCoords;

uniform sampler2D texture0;

void
main()
{
	gPosition = vec4(FragPos.xyz, 1);
	gNormal = vec4(0, 0, 1, 1);
	gAlbedoSpec = texture(texture0, TexCoords);
	gAlbedoSpec.a *= clamp(-FragPos.w * .0007, .1, 1.0);
}
