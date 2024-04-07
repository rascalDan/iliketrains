#version 330 core
#extension GL_ARB_shading_language_420pack : enable

in vec3 FragPos;
in vec2 TexCoords;
include(`materialOut.glsl')

uniform sampler2D texture0;

void
main()
{
	gPosition = ivec4(FragPos, 1);
	gNormal = vec4(0, 0, 1, 1);
	gAlbedoSpec = texture(texture0, TexCoords);
	gAlbedoSpec.a *= clamp(-FragPos.z * .0007, .1, 1.0);
}
