#version 460 core

layout(location = 0) in ivec3 position;

uniform ivec3 viewPoint;

out vec4 vworldPos;

void
main()
{
	vworldPos = vec4(position - viewPoint, 1);
}
