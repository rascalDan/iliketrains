#version 330 core

in vec2 texCoord0;

uniform sampler2D sampler;
uniform vec3 colour;

void
main()
{
	gl_FragColor = vec4(colour, texture(sampler, texCoord0).r);
}
