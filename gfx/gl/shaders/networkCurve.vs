#version 330 core

layout(location = 0) in ivec3 v_apos;
layout(location = 1) in ivec3 v_bpos;
layout(location = 2) in ivec3 v_centre;
layout(location = 3) in float v_reps;

uniform mat4 viewProjection;

flat out ivec3 apos;
flat out ivec3 bpos;
flat out mat2 rot;
flat out float reps;
flat out float dist;

void
main()
{
	dist = viewProjection[0][0];
}
