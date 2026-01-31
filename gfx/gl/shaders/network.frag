#version 460 core

include(`materialOut.glsl')
in vec3 rposition;
in vec2 texCoord;

layout(binding = 0) uniform sampler2D texture0;
uniform ivec3 viewPoint;

void
main()
{
	gPosition = ivec4(viewPoint + rposition, 1);
	gNormal = vec4(0, 0, 1, 1);
	gAlbedoSpec = texture(texture0, texCoord);
}
