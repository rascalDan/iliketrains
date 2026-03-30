#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "commonLocationData.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_direction;
layout(location = 2) in vec3 v_colour;
layout(location = 3) in float v_kq;
layout(location = 4) in float v_arc;
layout(location = 5) in uint index;
mat3 model = mat3(locations[cldIndex[index]].rotationMatrix);
ivec3 modelPos = locations[cldIndex[index]].position.xyz;

uniform ivec3 viewPoint;

flat out ivec3 position;
flat out vec3 direction;
flat out float size;
flat out vec2 arc; // cos,tan (v_arc/2)
flat out vec3 colour;
flat out float kq;

void
main()
{
	position = (modelPos - viewPoint) + ivec3(mat3(model) * v_position);
	direction = normalize(mat3(model) * v_direction);
	colour = v_colour;
	kq = v_kq;
	size = (8000 * sqrt(max(max(colour.r, colour.g), colour.b))) / sqrt(kq);
	arc = vec2(cos(v_arc / 2), tan(v_arc / 2));
	gl_Position = vec4(position, 0);
}
