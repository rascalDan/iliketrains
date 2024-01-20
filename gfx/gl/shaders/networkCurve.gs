#version 330 core

flat in ivec3 apos[];
flat in ivec3 bpos[];
flat in ivec3 cpos[];
flat in float reps[];
flat in float aangle[];
flat in float bangle[];

layout(points) in;
layout(triangle_strip, max_vertices = 255) out;

const mat2 rot = mat2(1);

include(`networkCommon.glsl')

mat2
getRot(float angle)
{
	return mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

void
main()
{
	float segs = floor(255 / (profile.length() * 2));
	float radius = distance(cpos[0], apos[0]);
	vec3 arcstep = vec3((bangle[0] - aangle[0]), // angle
						   reps[0], // texture
						   (bpos[0].z - apos[0].z)) // height
			/ segs;

	ivec3 prevPos = apos[0];
	mat2 prevRot = getRot(aangle[0]);
	float prevTex = 0;
	for (vec3 arc = arcstep; arc.y < reps[0] - 0.01; arc += arcstep) {
		mat2 rot = getRot(arc.x + aangle[0]);
		ivec3 pos = cpos[0] + ivec3(rot * vec2(radius, 0), arc.z);
		float tex = arc.y;
		doSeg(segDist(prevPos, pos), pos, prevPos, tex, prevTex, rot, prevRot);
		prevPos = pos;
		prevRot = rot;
		prevTex = tex;
	}
	doSeg(segDist(prevPos, bpos[0]), bpos[0], prevPos, reps[0], prevTex, getRot(bangle[0]), prevRot);
}
