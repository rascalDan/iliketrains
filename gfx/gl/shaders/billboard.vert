#version 460 core

uniform mat4 viewProjection;
uniform ivec4 viewPort;
uniform ivec3 viewPoint;
uniform vec3 centre;
uniform float size;
layout(location = 0) in ivec3 modelPos;
layout(location = 1) in float yaw;

flat out ivec3 ModelPos;
flat out float Yaw;
flat out float Depth;

void
main()
{
	ModelPos = modelPos;
	Yaw = yaw;
	gl_Position = viewProjection * vec4(modelPos - viewPoint + centre, 1);
	Depth = gl_Position.w;
	gl_PointSize = (viewPort.w * size * 2) / gl_Position.w;
}
