#version 460 core

layout(location = 0) in ivec3 position;
layout(location = 1) in vec3 normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 viewProjection;
uniform ivec3 viewPoint;

void
main()
{
	FragPos = position - viewPoint;
	Normal = normal;

	gl_Position = viewProjection * vec4(FragPos, 1);
}
