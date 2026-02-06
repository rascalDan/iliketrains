#version 460 core

layout(vertices = 1) out;

flat in ivec3 pos[][2];
flat in ivec2 cpos[];
flat in float reps[];
flat in float angles[][2];
flat in float radius[];

flat out ivec3 c_pos[][2];
flat out ivec2 c_cpos[];
flat out float c_reps[];
flat out float c_angles[][2];
flat out float c_radius[];

float
segments()
{
	const float arc = angles[gl_InvocationID][0] - angles[gl_InvocationID][1];
	const float error = 100.;
	const float diff = acos(1.f - (error / radius[gl_InvocationID]));
	return clamp(arc / diff, arc, 180);
}

void
main()
{
	c_pos[gl_InvocationID] = pos[gl_InvocationID];
	c_cpos[gl_InvocationID] = cpos[gl_InvocationID];
	c_reps[gl_InvocationID] = reps[gl_InvocationID];
	c_angles[gl_InvocationID] = angles[gl_InvocationID];
	c_radius[gl_InvocationID] = radius[gl_InvocationID];

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = segments();
}
