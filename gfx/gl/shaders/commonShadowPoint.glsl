out vec4 vworldPos;

void
main()
{
	vec3 worldPos = model * position;
	vworldPos = vec4(worldPos - viewPoint + modelPos, 1);
}
