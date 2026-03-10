#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "materialOut.glsl"

in vec3 rposition;
in vec2 texCoord;

layout(binding = 0) uniform sampler2D texture0;

void
main()
{
	gPosition = vec4(rposition, 1);
	gNormal = vec4(0, 0, 1, 1);
	gAlbedoSpec = texture(texture0, texCoord);
}
