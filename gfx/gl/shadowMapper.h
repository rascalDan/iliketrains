#pragma once

#include "lib/glArrays.h"
#include "program.h"
#include <glm/vec2.hpp>

class SceneProvider;
class Camera;

#include <gfx/models/texture.h>
class ShadowMapper {
public:
	explicit ShadowMapper(const glm::ivec2 & size);

	static constexpr std::size_t SHADOW_BANDS {4};
	struct Definitions {
		std::array<glm::mat4x4, SHADOW_BANDS> projections {};
		std::array<glm::vec4, SHADOW_BANDS> regions {};
		size_t maps {};
	};
	[[nodiscard]] Definitions update(const SceneProvider &, const glm::vec3 & direction, const Camera &) const;

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

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	operator GLuint() const
	{
		return depthMap;
	}

private:
	[[nodiscard]] static std::vector<std::array<glm::vec3, 4>> getBandViewExtents(
			const Camera &, const glm::mat4 & lightView);
	glFrameBuffer depthMapFBO;
	glTexture depthMap;
	glm::ivec2 size;
};
