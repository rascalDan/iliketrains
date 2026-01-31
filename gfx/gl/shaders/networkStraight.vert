#version 460 core

layout(location = 0) in ivec3 v_pos[2];
layout(location = 2) in mat2 v_rot;
layout(location = 4) in float v_reps;

flat out ivec3 pos[2];
flat out mat2 rot;
flat out float reps;
flat out float dist;

include(`networkCommon.glsl')

void
main()
{
	pos = v_pos;
	rot = v_rot;
	reps = v_reps;
	dist = segDist(v_pos[0], v_pos[1]);
}
