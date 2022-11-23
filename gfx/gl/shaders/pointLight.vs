#version 330 core

layout(location = 0) in vec3 position;

uniform vec3 colour;
uniform float kq;
uniform mat4 viewProjection;

out vec3 centre;
out float size;

void
main()
{
	centre = position;
	size = sqrt(256 * max(max(colour.r, colour.g), colour.b) / kq);
	gl_Position = viewProjection * vec4(centre, 1);
}
