#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`materialInterface.glsl')
include(`materialOut.glsl')

uniform sampler2D texture0;
uniform vec3 waves;

void
main()
{
	gPosition = vec4(FragPos, 1);
	gNormal = vec4(Normal, 1);
	gAlbedoSpec = texture(texture0, TexCoords);
	gAlbedoSpec.a *= clamp(-FragPos.z * .0007, .1, 1.0);
}
