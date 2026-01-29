#version 460 core

layout(location = 0) in ivec3 v_apos;
layout(location = 1) in ivec3 v_bpos;
layout(location = 2) in ivec3 v_centre;
layout(location = 3) in float v_reps;
layout(location = 4) in float v_aangle;
layout(location = 5) in float v_bangle;
layout(location = 6) in float v_radius;

flat out ivec3 apos;
flat out ivec3 bpos;
flat out ivec3 cpos;
flat out float reps;
flat out float aangle;
flat out float bangle;
flat out float radius;

void
main()
{
	apos = v_apos;
	bpos = v_bpos;
	cpos = v_centre;
	reps = v_reps;
	aangle = v_aangle;
	bangle = v_bangle;
	radius = v_radius;
}
