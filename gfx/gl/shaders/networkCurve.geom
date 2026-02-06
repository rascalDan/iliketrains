#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 10) out;

flat in ivec3 pos[];
flat in mat2 rot[];
flat in float tpos[];
flat in float dist[];

include(`networkCommon.glsl')

void
main()
{
	doSeg(min(dist[0], dist[1]), pos[0], pos[1], tpos[0], tpos[1], rot[0], rot[1]);
}
