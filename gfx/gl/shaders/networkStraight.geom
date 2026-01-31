#version 460 core

layout(points) in;
layout(triangle_strip, max_vertices = 10) out;

flat in ivec3 pos[][2];
flat in mat2 rot[];
flat in float reps[];
flat in float dist[];

include(`networkCommon.glsl')

void
main()
{
	doSeg(dist[0], pos[0][0], pos[0][1], 0.f, reps[0], rot[0], rot[0]);
}
