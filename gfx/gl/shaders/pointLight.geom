#version 460 core

const vec3[] cube = vec3[]( // http://www.cs.umd.edu/gvil/papers/av_ts.pdf
		vec3(-1, 1, 1), // Front-top-left
		vec3(1, 1, 1), // Front-top-right
		vec3(-1, -1, 1), // Front-bottom-left
		vec3(1, -1, 1), // Front-bottom-right
		vec3(1, -1, -1), // Back-bottom-right
		vec3(1, 1, 1), // Front-top-right
		vec3(1, 1, -1), // Back-top-right
		vec3(-1, 1, 1), // Front-top-left
		vec3(-1, 1, -1), // Back-top-left
		vec3(-1, -1, 1), // Front-bottom-left
		vec3(-1, -1, -1), // Back-bottom-left
		vec3(1, -1, -1), // Back-bottom-right
		vec3(-1, 1, -1), // Back-top-left
		vec3(1, 1, -1) // Back-top-right
);
uniform mat4 viewProjection;
uniform ivec3 viewPoint;
flat in vec3 position[];
flat in vec3 colour[];
flat in float size[];
flat in float kq[];
layout(points) in;

layout(triangle_strip, max_vertices = cube.length()) out;
flat out vec4 geo_centre;
flat out vec3 geo_colour;
flat out float geo_kq;

void
doVertex(int idx)
{
	gl_Position = viewProjection * (gl_in[0].gl_Position + vec4(cube[idx] * size[0], 1));
	geo_centre = vec4(position[0], size[0]);
	geo_colour = colour[0];
	geo_kq = kq[0];
	EmitVertex();
}

void
main()
{
	for (int i = 0; i < cube.length(); ++i) {
		doVertex(i);
	}
	EndPrimitive();
}
