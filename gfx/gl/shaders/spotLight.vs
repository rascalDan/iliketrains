#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_direction;
layout(location = 2) in vec3 v_colour;
layout(location = 3) in float v_kq;
layout(location = 4) in float v_arc;
layout(location = 5) in mat3 model;
layout(location = 8) in ivec3 modelPos;

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
	position = modelPos + ivec3(mat3(model) * v_position);
	direction = normalize(mat3(model) * v_direction);
	colour = v_colour;
	kq = v_kq;
	size = (8000 * sqrt(max(max(colour.r, colour.g), colour.b))) / sqrt(kq);
	arc = vec2(cos(v_arc / 2), tan(v_arc / 2));
	gl_Position = vec4(position - viewPoint, 0);
}
