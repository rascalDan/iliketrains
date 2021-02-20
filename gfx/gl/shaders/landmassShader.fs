#version 130

in vec2 texCoord0;
in vec3 normal0;
in float height;

uniform sampler2D sampler;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;

const vec3 grass = vec3(.1, .4, .05);
const vec3 sand = vec3(.76, .7, .5);
const vec3 snow = vec3(.97, .97, .99);

vec2
grad_between(float x, float lower, float upper)
{
	float off = (x - lower) / (upper - lower);
	return vec2(off, 1 - off);
}

void
main()
{
	gl_FragColor = texture(sampler, texCoord0);
	gl_FragColor.xyz *= clamp(ambientColor + (dot(-lightDirection, normal0) * lightColor), 0.0, 1.0);
	if (height < 0.5) {
		gl_FragColor.rgb *= sand;
	}
	else if (height > 30) {
		gl_FragColor.rgb *= snow;
	}
	else if (height > 28) {
		vec2 grad = grad_between(height, 28, 30);
		gl_FragColor.rgb *= grass + (snow - grass) * grad.x;
	}
	else {
		gl_FragColor.rgb *= grass;
	}
}
