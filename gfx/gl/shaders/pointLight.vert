#version 460 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_colour;
layout(location = 2) in float v_kq;
layout(location = 3) in mat3 model;
layout(location = 6) in ivec3 modelPos;

uniform ivec3 viewPoint;

flat out vec3 position;
flat out vec3 colour;
flat out float size;
flat out float kq;

void
main()
{
	position = modelPos + ivec3(mat3(model) * v_position);
	kq = v_kq;
	size = (8000 * sqrt(max(max(v_colour.r, v_colour.g), v_colour.b))) / sqrt(v_kq);
	colour = v_colour;
	gl_Position = vec4(position - viewPoint, 0);
}
