#version 460 core

layout(isolines, equal_spacing, cw) in;

uniform ivec3 viewPoint;

flat in ivec3 c_pos[][2];
flat in ivec2 c_cpos[];
flat in float c_reps[];
flat in float c_angles[][2];
flat in float c_radius[];

flat out ivec3 pos;
flat out mat2 rot;
flat out float tpos;
flat out float dist;

const float startTolerance = 1. / 200.;
const float endTolerance = 1. - startTolerance;

mat2
getRot(float angle)
{
	return mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

void
main()
{
	const float angle = mix(c_angles[0][1], c_angles[0][0], gl_TessCoord.x);
	rot = getRot(angle);
	if (gl_TessCoord.x < startTolerance) {
		pos = c_pos[0][1];
	}
	else if (gl_TessCoord.x > endTolerance) {
		pos = c_pos[0][0];
	}
	else {
		const int height = int(mix(c_pos[0][1].z, c_pos[0][0].z, gl_TessCoord.x));
		pos = ivec3(c_cpos[0] + ivec2(rot * vec2(c_radius[0], 0)), height);
	}

	tpos = c_reps[0] * gl_TessCoord.x;
	dist = length(vec3(viewPoint - pos));
}
