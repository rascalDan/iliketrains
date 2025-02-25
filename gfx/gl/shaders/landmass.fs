#version 330 core

include(`materialOut.glsl')
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture0;
uniform ivec3 viewPoint;
uniform vec3 colourBias;

const vec3 grass = vec3(.1, .4, .05);
const vec3 slope = vec3(.6, .6, .4);
const vec3 rock = vec3(.2, .2, .1);
const vec3 sand = vec3(.76, .7, .5);
const vec3 snow = vec3(.97, .97, .99);

const int beachline = 500;
const int snowline_low = 28000;
const int snowline_high = 30000;

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
	ivec3 position = ivec3(FragPos) + viewPoint;
	vec3 color = texture(texture0, vec2(position.xy % 10000) / 10000.0).rgb;

	int height = position.z;
	if (colourBias.r >= 0) {
		color *= colourBias;
	}
	else if (height < beachline) { // Sandy beach
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

	gPosition = ivec4(position, 1);
	gNormal = vec4(Normal, 1);
	gAlbedoSpec = vec4(color, 1);
}
