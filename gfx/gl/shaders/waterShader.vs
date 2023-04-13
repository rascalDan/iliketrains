#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform vec3 waves;

void
main()
{
	vec4 wpos = vec4(position.x + cos(waves.x), position.y + cos(waves.x * waves.y / 2),
			cos(waves.x + position.x + (position.y / 8)) * .3, 1.0);

	FragPos = vec3(wpos.xy, position.z);
	TexCoords = texCoord;
	Normal = normal;

	gl_Position = viewProjection * wpos;
}
