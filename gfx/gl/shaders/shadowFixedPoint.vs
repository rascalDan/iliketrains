#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 viewProjection;

void
main()
{
	gl_Position = viewProjection * vec4(position, 1.0);
	gl_Position.z = max(gl_Position.z, -1);
}
