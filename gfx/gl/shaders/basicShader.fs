#version 330 core

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec4 Colour;
include(`geometryOut.glsl')

uniform sampler2D texture0;

void
main()
{
	vec4 textureColour = texture(texture0, TexCoords);
	float clear = round(mix(textureColour.a, 1, Colour.a));
	gPosition = vec4(FragPos, clear);
	gNormal = vec4(Normal, clear);
	gAlbedoSpec = mix(textureColour, vec4(Colour.rgb, 1), Colour.a);
}
