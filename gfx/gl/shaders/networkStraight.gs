#version 330 core

flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in mat2 rot[];
flat in float reps[];
flat in float dist[];

layout(points) in;
layout(triangle_strip, max_vertices = 20) out;

void
main()
{
}
