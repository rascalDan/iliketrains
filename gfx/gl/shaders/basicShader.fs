#version 130

in vec2 texCoord0;
in vec3 normal0;

uniform sampler2D sampler;
uniform vec3 lightDirection;

void main()
{
	gl_FragColor = texture(sampler, texCoord0);
	gl_FragColor.xyz *= clamp(dot(-lightDirection, normal0), 0.0, 1.0);
}
