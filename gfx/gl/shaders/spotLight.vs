#version 330 core

layout(location = 0) in vec3 v_position;

uniform vec3 v_direction;
uniform vec3 colour;
uniform float kq;
uniform float arc;
uniform vec3 viewPoint;

out vec3 position;
out vec3 direction;
out float size;
out float cosarc;

void
main()
{
	position = v_position;
	direction = normalize(v_direction);
	size = (8 * sqrt(max(max(colour.r, colour.g), colour.b))) / sqrt(kq);
	cosarc = cos(arc / 2);
	gl_Position = vec4(position - viewPoint, 0);
}
