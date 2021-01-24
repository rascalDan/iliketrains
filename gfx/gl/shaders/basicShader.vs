#version 120

attribute vec3 position;
attribute vec2 texCoord;
attribute vec3 normal;

varying vec2 texCoord0;
varying vec3 normal0;

uniform mat4 viewProjection;
uniform mat4 model;

void main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	texCoord0 = texCoord;
	normal0 = (model * vec4(normal, 0.0)).xyz;
}
