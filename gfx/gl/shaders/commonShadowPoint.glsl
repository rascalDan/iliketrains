void
main()
{
	vec4 worldPos = model * vec4(position, 1.0);
	gl_Position = viewProjection * vec4(worldPos.xyz - viewPoint + modelPos, 1);
	gl_Position.z = max(gl_Position.z, -1);
}
