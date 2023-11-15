#version 330 core

include(`materialInterface.glsl')
include(`materialOut.glsl')

uniform sampler2D texture0;

const vec3 grass = vec3(.1, .4, .05);
const vec3 slope = vec3(.6, .6, .4);
const vec3 rock = vec3(.2, .2, .1);
const vec3 sand = vec3(.76, .7, .5);
const vec3 snow = vec3(.97, .97, .99);

const float beachline = 500;
const float snowline_low = 28000;
const float snowline_high = 30000;

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
	vec3 color = texture(texture0, TexCoords).rgb;

	float height = FragPos.z;
	if (height < beachline) { // Sandy beach
		color *= sand;
	}
	else if (Normal.z < slope_max) { // Dark rocky face
		color *= rock;
	}
	else { // Colour by lesser slope
		if (Normal.z < slope_mid) {
			color *= mixBetween(rock, slope, Normal.z, slope_max, slope_mid);
		}
		else if (Normal.z < slope_min) {
			color *= mixBetween(slope, grass, Normal.z, slope_mid, slope_min);
		}
		else {
			color *= grass;
		}

		// Add a snow covering
		if (height > snowline_low) {
			vec3 tsnow = color * snow;
			if (height > snowline_high) {
				color = tsnow;
			}
			else {
				color = mixBetween(color, tsnow, height, snowline_low, snowline_high);
			}
		}
	}

	gPosition = vec4(FragPos, 1);
	gNormal = vec4(Normal, 1);
	gAlbedoSpec = vec4(color, 1);
}
