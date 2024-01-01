#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
uniform float waves;

void
main()
{
	vec3 wpos = vec3(position.x + (cos(waves) * 1000.0), position.y + (cos(waves * 0 / 2) * 1000.0),
			cos(waves + (position.x / 1000000.0) + (position.y / 8000.0)) * 300.0);

	FragPos = vec3(wpos.xy, position.z);
	TexCoords = texCoord;
	Normal = normal;

	gl_Position = viewProjection * vec4(wpos - viewPoint, 1.0);
}
