#version 130

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 texCoord0;
out float depth;

uniform mat4 viewProjection;
uniform mat4 model;
uniform vec3 waves;

void main()
{
	vec3 wpos = vec3(
			position.x + cos(waves.x),
			cos(waves.x + position.x + (position.z / 7)) * .3,
			position.z + cos(waves.x * waves.z));
	gl_Position = viewProjection * model * vec4(wpos, 1.0);
	texCoord0 = texCoord;
	depth = position.y;
}