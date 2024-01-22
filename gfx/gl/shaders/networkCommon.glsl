uniform vec3[10] profile;
uniform float[10] texturePos;
uniform uint profileLength;

uniform mat4 viewProjection;
uniform ivec3 viewPoint;

uniform float clipDistance = 5000000;
uniform float flatDistance = 1000000;

out vec2 texCoord;
out vec3 rposition;

float
segDist(const ivec3 a, const ivec3 b)
{
	return min(distance(viewPoint, a), distance(viewPoint, b));
}

ifelse(
		TYPE, .gs,
		// Begin: Geometry shader only function
		void doVertex(const ivec3 end, const uint v, const float texY, const mat2 rot) {
			rposition = vec3(rot * profile[v].xy, profile[v].z);
			ivec3 vpos = end + ivec3(rposition);
			gl_Position = viewProjection * vec4(vpos - viewPoint, 1);
			texCoord = vec2(texturePos[v], texY);
			EmitVertex();
		}

		void doSeg(const float dist, const ivec3 apos, const ivec3 bpos, const float atexY, const float btexY,
				const mat2 arot, const mat2 brot) {
			if (dist < clipDistance) {
				uint vstep = (dist < flatDistance) ? 1u : profileLength - 1u;
				for (uint v = 0u; v < profileLength; v += vstep) {
					doVertex(bpos, v, btexY, brot);
					doVertex(apos, v, atexY, arot);
				}
				EndPrimitive();
			}
		}
		// End: Geometry shader only function
)
