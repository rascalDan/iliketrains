#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in ivec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 colourBias;

out vec3 FragPos;
out vec3 Normal;
flat out vec3 ColourBias;

uniform mat4 viewProjection;
uniform ivec3 viewPoint;

void
main()
{
	FragPos = position - viewPoint;
	Normal = normal;
	ColourBias = colourBias;

	gl_Position = viewProjection * vec4(FragPos, 1);
}
