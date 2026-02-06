#version 460 core

const vec3[] pyramid = vec3[]( // four-sided
		vec3(0, 0, 0), // Apex
		vec3(-1, 1, 1), // Back-left
		vec3(-1, -1, 1), // Front-left
		vec3(1, 1, 1), // Back-right
		vec3(1, -1, 1) // Front-right
);
uniform mat4 viewProjection;
uniform ivec3 viewPoint;
flat in ivec3 position[];
flat in vec3 direction[];
flat in vec3 colour[];
flat in float size[];
flat in float kq[];
flat in vec2 arc[];
layout(points) in;

layout(triangle_strip, max_vertices = 8) out;
flat out vec4 geo_centre;
flat out vec4 geo_direction;
flat out vec3 geo_colour;
flat out float geo_kq;

vec3
perp(vec3 a)
{
	return normalize(a.x != 0  ? vec3((a.y + a.z) / -a.x, 1, 1)
					: a.y != 0 ? vec3(1, (a.x + a.z) / -a.y, 1)
							   : vec3(1, 1, (a.x + a.y) / -a.z));
}

void
doVertex(vec4 ndcpos)
{
	gl_Position = ndcpos;
	geo_centre = vec4(position[0], size[0]);
	geo_direction = vec4(direction[0], arc[0].x);
	geo_colour = colour[0];
	geo_kq = kq[0];
	EmitVertex();
}

void
main()
{
	const float base = size[0] * arc[0].y;
	const vec3 offx = perp(direction[0]);
	const vec3 offy = cross(direction[0], offx);
	vec4 out_py[pyramid.length()];
	for (int i = 0; i < pyramid.length(); ++i) {
		const vec3 p = pyramid[i];
		const vec3 edge = (offx * base * p.x) + (offy * base * p.y) + (direction[0] * size[0] * p.z);
		out_py[i] = viewProjection * (gl_in[0].gl_Position + vec4(edge, 1));
	}
	doVertex(out_py[3]);
	doVertex(out_py[0]);
	doVertex(out_py[1]);
	doVertex(out_py[2]);
	doVertex(out_py[3]);
	doVertex(out_py[4]);
	doVertex(out_py[0]);
	doVertex(out_py[2]);
	EndPrimitive();
}
