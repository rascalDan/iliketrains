#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2DArray stencilDepth;
flat in vec3 scale;
in vec3 texCoord;

void
main()
{
	float stDepth = texture(stencilDepth, texCoord).r;
	if (stDepth >= 1) {
		discard;
	}
	gl_FragDepth = gl_FragCoord.z + (stDepth * scale.z);
}
