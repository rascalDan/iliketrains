#version 460 core

in vec4 FragPos;
in vec2 TexCoords;
include(`materialOut.glsl')

uniform sampler2D texture0;

void
main()
{
	gPosition = vec4(FragPos.xyz, 1);
	gNormal = vec4(0, 0, 1, 1);
	gAlbedoSpec = texture(texture0, TexCoords);
	gAlbedoSpec.a *= clamp(-FragPos.w * .0007, .1, 1.0);
}
