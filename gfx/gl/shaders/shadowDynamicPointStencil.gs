#version 330 core
#extension GL_ARB_viewport_array : enable

const vec2[] corners = vec2[4](vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, 1));
const float tau = 6.28318531;

uniform mat4 viewProjection[4];
uniform int viewProjections;
uniform vec3 sizes[4];
uniform float size;

in float vmodelYaw[];
in ivec3 vworldPos[];

flat out vec3 scale;
out vec3 texCoord;

layout(points) in;
layout(triangle_strip, max_vertices = 16) out;

void
main()
{
	int viewAngle = int(round(4.0 + (vmodelYaw[0] / tau))) % 8;
	for (gl_Layer = 0; gl_Layer < viewProjections; ++gl_Layer) {
		scale = 2.0 * size / sizes[gl_Layer];
		vec4 pos = viewProjection[gl_Layer] * vec4(vworldPos[0], 1);
		for (int c = 0; c < corners.length(); ++c) {
			gl_Position = pos + vec4(scale.xy * corners[c], 0, 0);
			gl_Position.z = max(gl_Position.z, -1);
			texCoord = vec3((corners[c] * 0.5) + 0.5, viewAngle);
			EmitVertex();
		}
		EndPrimitive();
	}
}
