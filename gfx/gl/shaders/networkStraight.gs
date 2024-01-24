#version 330 core

flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in mat2 rot[];
flat in float reps[];
flat in float dist[];

layout(points) in;
layout(triangle_strip, max_vertices = 10) out;
include(`networkCommon.glsl')

void
main()
{
	doSeg(dist[0], apos[0], bpos[0], 0.f, reps[0], rot[0], rot[0]);
}
