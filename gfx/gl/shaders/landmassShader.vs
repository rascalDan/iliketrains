#version 130

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 texCoord0;
out vec3 normal0;
out float height;

uniform mat4 viewProjection;
uniform mat4 model;
uniform vec3 waves;

void main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	texCoord0 = texCoord;
	normal0 = normal;
	height = position.y;
}