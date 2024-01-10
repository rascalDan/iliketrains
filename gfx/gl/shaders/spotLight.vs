#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_direction;
// layout(location = 2) in vec3 colour;
// layout(location = 3) in float kq;
// layout(location = 4) in float arc;
layout(location = 5) in mat4 model;
layout(location = 9) in ivec3 modelPos;

const vec3 colour = vec3(1);
const float kq = 0.01;
const float arc = 1;

uniform ivec3 viewPoint;

out ivec3 position;
out vec3 direction;
out float size;
out float cosarc;

void
main()
{
	position = modelPos + ivec3(v_position * mat3(model));
	direction = normalize(v_direction * mat3(model));
	size = (8000 * sqrt(max(max(colour.r, colour.g), colour.b))) / sqrt(kq);
	cosarc = cos(arc / 2);
	gl_Position = vec4(position - viewPoint, 0);
}
