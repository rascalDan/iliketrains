#version 460 core
#extension GL_ARB_shading_language_include : enable

const float tau = 6.28318531;

layout(binding = 0) uniform sampler2DArray billboardDepth;
layout(binding = 1) uniform sampler2DArray billboardNormal;
layout(binding = 2) uniform sampler2DArray billboardAlbedo;
uniform mat4 viewProjection;
uniform float size;

#include "materialOut.glsl"

flat in vec3 ModelPos;
flat in float Yaw;
flat in float Depth;

void
main()
{
	int viewAngle = int(round(8 * Yaw / tau)) % 8;
	vec3 texel = vec3(gl_PointCoord * vec2(-1, 1) + vec2(1, 0), viewAngle);
	gAlbedoSpec = texture(billboardAlbedo, texel);
	if (gAlbedoSpec.a < 0.5) {
		discard;
	}
	gPosition = ivec4(ModelPos + vec3(0, 0, size * 2 * (1 - gl_PointCoord.y)), 1);
	gNormal = texture(billboardNormal, texel) * vec4(-1, -1, 1, 1);
}
