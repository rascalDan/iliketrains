#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform vec3 viewPoint;
uniform vec3 waves;

void
main()
{
	vec3 wpos = vec3(position.x + cos(waves.x), position.y + cos(waves.x * waves.y / 2),
			cos(waves.x + position.x + (position.y / 8)) * .3);

	FragPos = vec3(wpos.xy, position.z);
	TexCoords = texCoord;
	Normal = normal;

	gl_Position = viewProjection * vec4(wpos - viewPoint, 1.0);
}
