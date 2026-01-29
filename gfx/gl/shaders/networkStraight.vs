#version 460 core

layout(location = 0) in ivec3 v_apos;
layout(location = 1) in ivec3 v_bpos;
layout(location = 2) in mat2 v_rot;
layout(location = 4) in float v_reps;

flat out ivec3 apos;
flat out ivec3 bpos;
flat out mat2 rot;
flat out float reps;
flat out float dist;

include(`networkCommon.glsl')

void
main()
{
	apos = v_apos;
	bpos = v_bpos;
	rot = v_rot;
	reps = v_reps;
	dist = segDist(v_apos, v_bpos);
}
