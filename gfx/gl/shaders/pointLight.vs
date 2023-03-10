#version 330 core

layout(location = 0) in vec3 position;

uniform vec3 colour;
uniform float kq;

out vec3 centre;
out float size;

void
main()
{
	centre = position;
	size = (8 * sqrt(max(max(colour.r, colour.g), colour.b))) / sqrt(kq);
	gl_Position = vec4(centre, 0);
}
