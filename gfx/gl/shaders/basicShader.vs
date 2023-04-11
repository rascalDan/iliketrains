#version 330 core

include(`meshIn.glsl')
include(`materialInterface.glsl')

uniform mat4 viewProjection;
uniform mat4 model;

void
main()
{
	vec4 worldPos = model * vec4(position, 1.0);

	FragPos = worldPos.xyz;
	TexCoords = texCoord;
	Normal = (model * vec4(normal, 0.0)).xyz;
	Colour = colour;

	gl_Position = viewProjection * worldPos;
}
