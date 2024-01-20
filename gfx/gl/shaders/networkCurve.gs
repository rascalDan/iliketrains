#version 330 core

flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in ivec3 cpos[];
flat in float reps[];

layout(points) in;
layout(triangle_strip, max_vertices = 255) out;

const mat2 rot = mat2(1);

include(`networkCommon.glsl')

void
main()
{
	doSeg(0, apos[0], bpos[0], 0.f, reps[0], rot, rot);
}
