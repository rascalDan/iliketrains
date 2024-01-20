const float RAIL_HEIGHT = 250;
const vec3[] profile = vec3[]( //
		vec3(-1900.F, 0.F, 0.F), //
		vec3(-608.F, 0.F, RAIL_HEIGHT), //
		vec3(0, 0.F, RAIL_HEIGHT * .7F), //
		vec3(608.F, 0.F, RAIL_HEIGHT), //
		vec3(1900.F, 0.F, 0.F));
const float[profile.length()] texturePos = float[](0, 0.34, 0.5, 0.65, 1);

uniform mat4 viewProjection;
uniform ivec3 viewPoint;

uniform float clipDistance = 5000000;
uniform float flatDistance = 1000000;

out vec2 texCoord;
out vec3 rposition;

void
doVertex(const ivec3 end, const int v, const float texY, const mat2 rot)
{
	rposition = vec3(rot * profile[v].xy, profile[v].z);
	ivec3 vpos = end + ivec3(rposition);
	gl_Position = viewProjection * vec4(vpos - viewPoint, 1);
	texCoord = vec2(texturePos[v], texY);
	EmitVertex();
}

void
doSeg(const float dist, const ivec3 apos, const ivec3 bpos, const float atexY, const float btexY, const mat2 arot,
		const mat2 brot)
{
	if (dist < clipDistance) {
		int vstep = (dist < flatDistance) ? 1 : profile.length() - 1;
		for (int v = 0; v < profile.length(); v += vstep) {
			doVertex(bpos, v, btexY, brot);
			doVertex(apos, v, atexY, arot);
		}
		EndPrimitive();
	}
}
