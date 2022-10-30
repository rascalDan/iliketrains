#version 330 core

in vec2 texCoord0;
in vec3 normal0;
in float height;

uniform sampler2D sampler;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;

const vec3 grass = vec3(.1, .4, .05);
const vec3 slope = vec3(.6, .6, .4);
const vec3 rock = vec3(.2, .2, .1);
const vec3 sand = vec3(.76, .7, .5);
const vec3 snow = vec3(.97, .97, .99);

const float beachline = .5;
const float snowline_low = 28;
const float snowline_high = 30;

const float slope_min = .99;
const float slope_mid = .95;
const float slope_max = .90;

vec3
mixBetween(vec3 colA, vec3 colB, float blend, float low, float high)
{
	return mix(colA, colB, (blend - low) / (high - low));
}

void
main()
{
	vec4 tex = texture(sampler, texCoord0);
	gl_FragColor = tex;
	gl_FragColor.rgb *= clamp(ambientColor + (dot(-lightDirection, normal0) * lightColor), 0.0, 1.0);

	if (height < beachline) { // Sandy beach
		gl_FragColor.rgb *= sand;
	}
	else if (normal0.z < slope_max) { // Dark rocky face
		gl_FragColor.rgb *= rock;
	}
	else { // Colour by lesser slope
		if (normal0.z < slope_mid) {
			gl_FragColor.rgb *= mixBetween(rock, slope, normal0.z, slope_max, slope_mid);
		}
		else if (normal0.z < slope_min) {
			gl_FragColor.rgb *= mixBetween(slope, grass, normal0.z, slope_mid, slope_min);
		}
		else {
			gl_FragColor.rgb *= grass;
		}

		// Add a snow covering
		if (height > snowline_low) {
			vec3 tsnow = tex.rgb * snow;
			if (height > snowline_high) {
				gl_FragColor.rgb = tsnow;
			}
			else {
				gl_FragColor.rgb = mixBetween(gl_FragColor.rgb, tsnow, height, snowline_low, snowline_high);
			}
		}
	}
}
