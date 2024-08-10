include(`materialDetail.glsl')

define(INOUT, ifelse(TYPE, .fs, in, out));

INOUT vec3 FragPos;
INOUT vec2 TexCoords;
INOUT vec3 Normal;
INOUT vec4 Colour;
flat INOUT MaterialDetail Material;
