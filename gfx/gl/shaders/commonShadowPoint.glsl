out vec4 vworldPos;

ifdef(`TEXTURES', out vec2 vtexCoord;);

void
main()
{
	vec3 worldPos = model * position;
	vworldPos = vec4(worldPos - viewPoint + modelPos, 1);
	ifdef(`TEXTURES', vtexCoord = texCoord;);
}
