#version 330 core

in vec2 texCoord0;
in float depth;

uniform sampler2D sampler;
uniform vec3 waves;

void main()
{
	gl_FragColor = texture(sampler, texCoord0);
	gl_FragColor.a *= clamp(-depth * .7, .1, 1.0);
}
