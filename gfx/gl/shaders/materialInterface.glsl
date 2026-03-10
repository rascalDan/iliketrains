#ifndef MATERIAL_INTERFACE_INCLUDED
#define MATERIAL_INTERFACE_INCLUDED

#include "materialDetail.glsl"

#ifdef GL_FRAGMENT_SHADER
#	define INOUT in
#else
#	define INOUT out
#endif

INOUT vec3 FragPos;
INOUT vec2 TexCoords;
INOUT vec3 Normal;
INOUT vec4 Colour;
flat INOUT MaterialDetail Material;

#endif
