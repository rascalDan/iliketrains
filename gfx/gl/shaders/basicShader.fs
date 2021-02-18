#version 130

in vec2 texCoord0;
in vec3 normal0;

uniform sampler2D sampler;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;

void main()
{
	gl_FragColor = texture(sampler, texCoord0);
	gl_FragColor.xyz *= clamp(ambientColor + (dot(-lightDirection, normal0) * lightColor), 0.0, 1.0);
}
