#version 460 core
layout(isolines, equal_spacing, cw) in;

flat in ivec3 c_apos[];
flat in ivec3 c_bpos[];
flat in ivec3 c_cpos[];
flat in float c_reps[];
flat in float c_aangle[];
flat in float c_bangle[];
flat in float c_radius[];

flat out ivec3 pos;
flat out mat2 rot;
flat out float tpos;

mat2
getRot(float angle)
{
	return mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

void
main()
{
	float angle = mix(c_bangle[0], c_aangle[0], gl_TessCoord.x);
	int height = int(mix(c_bpos[0].z, c_apos[0].z, gl_TessCoord.x)) - c_cpos[0].z;
	rot = getRot(angle);
	tpos = c_reps[0] * gl_TessCoord.x;
	pos = c_cpos[0] + ivec3(rot * vec2(c_radius[0], 0), height);
}
