#pragma once

#include "config/types.h"
#include "lib/glArrays.h"
#include "program.h"
#include <gfx/models/texture.h>
#include <glm/vec2.hpp>
#include <vector>

class SceneProvider;
class Camera;

class ShadowMapper {
public:
	explicit ShadowMapper(const TextureAbsCoord & size);

	static constexpr std::size_t SHADOW_BANDS {4};

	struct Definitions {
		std::array<glm::mat4x4, SHADOW_BANDS> projections {};
		std::array<TextureRelRegion, SHADOW_BANDS> regions {};
		size_t maps {};
	};

	[[nodiscard]] Definitions update(const SceneProvider &, const Direction3D & direction, const Camera &) const;

	class ShadowProgram : public Program {
	public:
		explicit ShadowProgram(const Shader & vs);

		void setViewPoint(const GlobalPosition3D, size_t n) const;
		void setViewProjection(const glm::mat4 &, size_t n) const;
		void use() const;

	private:
		std::array<RequiredUniformLocation, 4> viewProjectionLoc;
		RequiredUniformLocation viewProjectionsLoc;
		RequiredUniformLocation viewPointLoc;
	};

	class FixedPoint : public ShadowProgram {
	public:
		explicit FixedPoint(const Shader & vs);
	};

	class DynamicPoint : public ShadowProgram {
	public:
		DynamicPoint();
		void use(const Location &) const;
		void setModel(const Location &) const;

	private:
		RequiredUniformLocation modelLoc;
		RequiredUniformLocation modelPosLoc;
	};

	FixedPoint fixedPoint, dynamicPointInst;
	DynamicPoint dynamicPoint;

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	operator GLuint() const
	{
		return depthMap;
	}

private:
	[[nodiscard]] static std::vector<std::array<RelativePosition3D, 4>> getBandViewExtents(
			const Camera &, const glm::mat4 & lightView);
	glFrameBuffer depthMapFBO;
	glTexture depthMap;
	TextureAbsCoord size;
};
