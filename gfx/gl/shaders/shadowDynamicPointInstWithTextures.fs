#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 3) uniform sampler2D texture0;

in vec2 texCoord;

void
main()
{
	if (texture(texture0, texCoord).a < 0.5) {
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}
