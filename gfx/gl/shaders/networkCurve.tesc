#version 460 core

layout(vertices = 1) out;
flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in ivec3 cpos[];
flat in float reps[];
flat in float aangle[];
flat in float bangle[];
flat in float radius[];

flat out ivec3 c_apos[];
flat out ivec3 c_bpos[];
flat out ivec3 c_cpos[];
flat out float c_reps[];
flat out float c_aangle[];
flat out float c_bangle[];
flat out float c_radius[];

int
min2pow(float target)
{
	int x = 1;
	while (x < target) {
		x <<= 1;
	}
	return x;
}

void
main()
{
	c_apos[gl_InvocationID] = apos[gl_InvocationID];
	c_bpos[gl_InvocationID] = bpos[gl_InvocationID];
	c_cpos[gl_InvocationID] = cpos[gl_InvocationID];
	c_reps[gl_InvocationID] = reps[gl_InvocationID];
	c_aangle[gl_InvocationID] = aangle[gl_InvocationID];
	c_bangle[gl_InvocationID] = bangle[gl_InvocationID];
	c_radius[gl_InvocationID] = radius[gl_InvocationID];

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = min2pow(reps[gl_InvocationID]);
}
