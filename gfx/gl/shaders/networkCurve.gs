#version 460 core

flat in ivec3 pos[];
flat in mat2 rot[];
flat in float tpos[];

layout(lines) in;
layout(triangle_strip, max_vertices = 10) out;
include(`networkCommon.glsl')

void
main()
{
	doSeg(segDist(pos[0], pos[1]), pos[0], pos[1], tpos[0], tpos[1], rot[0], rot[1]);
}
