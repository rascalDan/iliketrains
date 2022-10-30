#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 texCoord0;
out vec3 normal0;

uniform mat4 viewProjection;
uniform mat4 model;

void main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	texCoord0 = texCoord;
	normal0 = (model * vec4(normal, 0.0)).xyz;
}
