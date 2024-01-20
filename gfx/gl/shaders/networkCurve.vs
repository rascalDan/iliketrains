#version 330 core

layout(location = 0) in ivec3 v_apos;
layout(location = 1) in ivec3 v_bpos;
layout(location = 2) in ivec3 v_centre;
layout(location = 3) in float v_reps;

flat out ivec3 apos;
flat out ivec3 bpos;
flat out ivec3 cpos;
flat out float reps;

void
main()
{
	apos = v_apos;
	bpos = v_bpos;
	cpos = v_centre;
	reps = v_reps;
}
