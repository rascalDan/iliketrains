#version 130

in vec2 texCoord0;
in vec3 normal0;
in float height;

uniform sampler2D sampler;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;

const vec3 grass = vec3(.1, .4, .05);
const vec3 sand = vec3(.76, .7, .5);
const vec3 snow = vec3(.97, .97, .99);

const float beachline = .5;
const float snowline_low = 28;
const float snowline_high = 30;

void
main()
{
	gl_FragColor = texture(sampler, texCoord0);
	gl_FragColor.rgb *= clamp(ambientColor + (dot(-lightDirection, normal0) * lightColor), 0.0, 1.0);
	if (height < beachline) {
		gl_FragColor.rgb *= sand;
	}
	else if (height > snowline_high) {
		gl_FragColor.rgb *= snow;
	}
	else if (height > snowline_low) {
		gl_FragColor.rgb *= mix(grass, snow, (height - snowline_low) / (snowline_high - snowline_low));
	}
	else {
		gl_FragColor.rgb *= grass;
	}
}
