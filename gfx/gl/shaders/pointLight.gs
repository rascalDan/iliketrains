#version 330 core
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shading_language_420pack : enable

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
uniform vec3 viewPoint;

in vec3 centre[];
in float size[];
layout(points) in;

layout(triangle_strip, max_vertices = cube.length()) out;
out vec4 geo_centre;

void
doVertex(int idx)
{
	gl_Position = viewProjection * (gl_in[0].gl_Position + vec4(cube[idx] * size[0], 1));
	geo_centre = vec4(centre[0], size[0]);
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
