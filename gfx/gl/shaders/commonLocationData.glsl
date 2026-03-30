#ifndef COMMON_LOCATION_DATA_INCLUDED
#define COMMON_LOCATION_DATA_INCLUDED

struct CommonLocationData {
	ivec4 position;
	vec4 rotation;
	mat3x4 rotationMatrix;
};

layout(binding = 0, std430) restrict readonly buffer commonLocationData
{
	CommonLocationData locations[];
};

layout(binding = 1, std430) restrict readonly buffer commonLocationDataIndex
{
	uint cldIndex[];
};

#endif
