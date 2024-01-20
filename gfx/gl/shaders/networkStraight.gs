#version 330 core

const float RAIL_HEIGHT = 250;
const vec3[] profile = vec3[]( //
		vec3(-1900.F, 0.F, 0.F), //
		vec3(-608.F, 0.F, RAIL_HEIGHT), //
		vec3(0, 0.F, RAIL_HEIGHT * .7F), //
		vec3(608.F, 0.F, RAIL_HEIGHT), //
		vec3(1900.F, 0.F, 0.F));
const float[profile.length()] texturePos = float[](0, 0.34, 0.5, 0.65, 1);

flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in mat2 rot[];
flat in float reps[];
flat in float dist[];

layout(points) in;
layout(triangle_strip, max_vertices = 10) out;

uniform mat4 viewProjection;
uniform ivec3 viewPoint;

uniform int clipDistance = 5000000;
uniform int flatDistance = 1000000;

out vec2 texCoord;
out vec3 rposition;

void
doEnd(const ivec3 end, int v, float texY)
{
	rposition = vec3(rot[0] * profile[v].xy, profile[v].z);
	ivec3 vpos = end + ivec3(rposition);
	gl_Position = viewProjection * vec4(vpos - viewPoint, 1);
	texCoord = vec2(texturePos[v], texY);
	EmitVertex();
}

void
main()
{
	if (dist[0] < clipDistance) {
		int vstep = (dist[0] < flatDistance) ? 1 : profile.length() - 1;
		for (int v = 0; v < profile.length(); v += vstep) {
			doEnd(bpos[0], v, reps[0]);
			doEnd(apos[0], v, 0);
		}
		EndPrimitive();
	}
}
