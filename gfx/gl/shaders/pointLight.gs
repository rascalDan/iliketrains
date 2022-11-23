#version 330 core
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shading_language_420pack : enable

const int maxv = 128;
const int maxarcs = (maxv / 2) - 1;
const int minarcs = 10;
const float tau = radians(360);
const float scale = 150;
uniform ivec4 viewPort;

in vec3 centre[];
in float size[];
layout(points) in;

layout(triangle_strip, max_vertices = maxv) out;
out vec3 geo_centre;

void
doSeg(float arc, vec2 radius)
{
	gl_Position = gl_in[0].gl_Position;
	geo_centre = centre[0];
	EmitVertex();

	const vec2 off = vec2(cos(arc), sin(arc)) * radius;
	gl_Position.xy = gl_in[0].gl_Position.xy + off;
	geo_centre = centre[0];
	EmitVertex();
}

void
main()
{
	const vec2 display = viewPort.zw;
	const vec2 ratio = vec2(1, display.x / display.y);
	const vec2 radius = (size[0] * ratio * scale) / gl_in[0].gl_Position.w;
	const float step = tau / clamp(radius.x, minarcs, maxarcs);
	for (float arc = 0; arc < tau; arc += step) {
		doSeg(arc, radius);
	}
	doSeg(tau, radius);
	EndPrimitive();
}
