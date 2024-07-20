#version 330 core
#extension GL_ARB_shading_language_420pack : enable

include(`materialInterface.glsl')
include(`materialOut.glsl')
include(`materialCommon.glsl')

void
main()
{
	vec4 textureColour = getTextureColour(Material, TexCoords);
	float opaque = step(0.5, mix(textureColour.a, 1, Colour.a));
	gPosition = ivec4(FragPos, opaque);
	gNormal = vec4(Normal, opaque);
	gl_FragDepth = mix(1.0, gl_FragCoord.z, opaque);
	gAlbedoSpec = mix(textureColour, vec4(Colour.rgb, 1), Colour.a);
}
