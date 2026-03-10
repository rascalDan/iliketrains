#version 460 core

layout(location = 0) in ivec3 position;
out vec4 FragPos;
out vec2 TexCoords;

uniform mat4 viewProjection;
uniform ivec3 viewPoint;
uniform float waves;

void
main()
{
	vec3 wpos = vec3(position.x + (cos(waves) * 1000.0), position.y + (cos(waves * 1.4) * 1000.0),
			cos(waves + (position.x / 1000000) + (position.y / 8000)) * 300.0);

	FragPos = vec4(wpos - viewPoint, position.z);
	TexCoords = (position.xy / 8192) - (viewPoint.xy / 8192);

	gl_Position = viewProjection * vec4(FragPos.xyz, 1.0);
}
