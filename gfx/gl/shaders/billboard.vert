#version 460 core
#extension GL_ARB_shading_language_include : enable

#include "commonLocationData.glsl"

uniform mat4 viewProjection;
uniform ivec4 viewPort;
uniform ivec3 viewPoint;
uniform vec3 centre;
uniform float size;

layout(location = 0) in uint index;

flat out vec3 ModelPos;
flat out float Yaw;
flat out float Depth;

void
main()
{
	const ivec3 modelPos = locations[cldIndex[index]].position.xyz;
	ModelPos = modelPos - viewPoint;
	Yaw = locations[cldIndex[index]].rotation.x;
	gl_Position = viewProjection * vec4(ModelPos + centre, 1);
	Depth = gl_Position.w;
	gl_PointSize = (viewPort.w * size * 2) / gl_Position.w;
}
