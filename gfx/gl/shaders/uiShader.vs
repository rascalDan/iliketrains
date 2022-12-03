#version 330 core

in vec4 position;

out vec2 texCoord0;
uniform mat4 uiProjection;

void
main()
{
	gl_Position = uiProjection * vec4(position.xy, 0.0, 1.0);
	texCoord0 = position.zw;
}
