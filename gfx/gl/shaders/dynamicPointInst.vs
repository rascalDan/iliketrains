#version 330 core

include(`meshIn.glsl')
layout(location = 5) in mat4 model;
include(`materialInterface.glsl')

uniform mat4 viewProjection;

void
main()
{
	vec4 worldPos = model * vec4(position, 1.0);

	FragPos = worldPos.xyz;
	TexCoords = texCoord;
	Normal = (model * vec4(normal, 0.0)).xyz;
	Colour = colour;
	Material = material;

	gl_Position = viewProjection * worldPos;
}
