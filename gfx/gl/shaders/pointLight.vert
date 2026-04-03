#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "commonLocationData.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_colour;
layout(location = 2) in float v_kq;
layout(location = 3) in uint index;
mat3 model = mat3(locations[cldIndex[index]].rotationMatrix);
ivec3 modelPos = locations[cldIndex[index]].position.xyz;

uniform ivec3 viewPoint;

flat out vec3 position;
flat out vec3 colour;
flat out float size;
flat out float kq;

void
main()
{
	position = (modelPos - viewPoint) + ivec3(mat3(model) * v_position);
	kq = v_kq;
	size = (8000 * sqrt(max(max(v_colour.r, v_colour.g), v_colour.b))) / sqrt(v_kq);
	colour = v_colour;
	gl_Position = vec4(position, 0);
}
