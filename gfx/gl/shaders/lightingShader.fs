#version 330 core
#extension GL_ARB_shading_language_420pack : enable

out vec3 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedoSpec;

// struct Light {
// vec3 Position;
// vec3 Color;

// float Linear;
// float Quadratic;
// float Radius;
//};
// const int NR_LIGHTS_MAX = 12;
// uniform Light lights[NR_LIGHTS_MAX];
// uniform vec3 viewPos;

uniform vec3 lightDirection = normalize(vec3(1, 0, -1));
uniform vec3 lightColor = vec3(0.6, 0.6, 0.6);
uniform vec3 ambientColor = vec3(0.5, 0.5, 0.5);

void
main()
{
	// retrieve data from gbuffer
	// const vec3 FragPos = texture(gPosition, TexCoords).rgb;
	const vec3 Normal = texture(gNormal, TexCoords).rgb;
	const vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
	// float Specular = texture(gAlbedoSpec, TexCoords).a;

	// then calculate lighting as usual
	// vec3 lighting = Diffuse;
	const vec3 lighting = ambientColor + clamp((dot(-lightDirection, Normal) * lightColor), 0, 0.5);
	// vec3 viewDir = normalize(viewPos - FragPos);
	// for (int i = 0; i < NR_LIGHTS_MAX; ++i) {
	//  calculate distance between light source and current fragment
	// float distance = length(lights[i].Position - FragPos);
	// if (distance < lights[i].Radius) {
	//  diffuse
	// vec3 lightDir = normalize(lights[i].Position - FragPos);
	// vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
	//  specular
	// vec3 halfwayDir = normalize(lightDir + viewDir);
	// float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
	// vec3 specular = lights[i].Color * spec * Specular;
	//  attenuation
	// float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
	// diffuse *= attenuation;
	// specular *= attenuation;
	// lighting += diffuse + specular;
	//}
	//}
	// FragColor = vec4(lighting, 1.0);

	FragColor = vec3(Diffuse * lighting);
}
