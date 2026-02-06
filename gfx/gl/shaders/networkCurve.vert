#version 460 core

layout(location = 0) in ivec3 v_pos[2];
layout(location = 2) in ivec3 v_centre;
layout(location = 3) in float v_reps;
layout(location = 4) in float v_angles[2];
layout(location = 6) in float v_radius;

flat out ivec3 pos[2];
flat out ivec2 cpos;
flat out float reps;
flat out float angles[2];
flat out float radius;

void
main()
{
	pos = v_pos;
	cpos = v_centre.xy;
	reps = v_reps;
	angles = v_angles;
	radius = v_radius;
}
