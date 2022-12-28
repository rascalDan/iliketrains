#pragma once

#include "lib/glArrays.h"
#include "program.h"
#include <glm/vec2.hpp>

class SceneProvider;
class Camera;

class ShadowMapper {
public:
	ShadowMapper(const glm::ivec2 & size);

	template<std::size_t S> struct Definitions {
		std::array<glm::mat4x4, S> projections;
		std::array<glm::vec4, S> regions;
	};
	Definitions<1> update(const SceneProvider &, const glm::vec3 & direction, const Camera &) const;

	class FixedPoint : public Program {
	public:
		FixedPoint();
		void setViewProjection(const glm::mat4 &) const;
		void use() const;

	private:
		RequiredUniformLocation viewProjectionLoc;
	};
	class DynamicPoint : public Program {
	public:
		DynamicPoint();
		void setViewProjection(const glm::mat4 &) const;
		void use(const Location &) const;

	private:
		RequiredUniformLocation viewProjectionLoc;
		RequiredUniformLocation modelLoc;
	};
	FixedPoint fixedPoint;
	DynamicPoint dynamicPoint;

	operator GLuint() const
	{
		return depthMap;
	}

private:
	glFrameBuffer depthMapFBO;
	glTexture depthMap;
	glm::ivec2 size;
};
