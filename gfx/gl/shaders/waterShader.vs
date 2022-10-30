#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 texCoord0;
out float depth;

uniform mat4 viewProjection;
uniform vec3 waves;

void main()
{
	vec3 wpos = vec3(
			position.x + cos(waves.x),
			position.y + cos(waves.x * waves.y / 2),
			cos(waves.x + position.x + (position.y / 8)) * .3);
	gl_Position = viewProjection * vec4(wpos, 1.0);
	texCoord0 = texCoord;
	depth = position.z;
}
