void
main()
{
	vec3 worldPos = model * position;
	gl_Position = viewProjection * vec4(worldPos - viewPoint + modelPos, 1);
	gl_Position.z = max(gl_Position.z, -1);
}
