#pragma once

#include "config/types.h"
#include "lib/glArrays.h"
#include "program.h"
#include <gfx/models/texture.h>
#include <glm/vec2.hpp>
#include <span>
#include <vector>

class SceneProvider;
class Camera;

class ShadowMapper {
public:
	explicit ShadowMapper(const TextureAbsCoord & size);

	static constexpr std::size_t SHADOW_BANDS {4};

	using Definitions = std::vector<glm::mat4x4>;

	[[nodiscard]] Definitions update(const SceneProvider &, const Direction3D & direction, const Camera &) const;

	class ShadowProgram : public Program {
	public:
		explicit ShadowProgram(const Shader & vs);

		void setView(const std::span<const glm::mat4>, const GlobalPosition3D) const;
		void use() const;

	private:
		RequiredUniformLocation viewProjectionLoc;
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

	FixedPoint landmess, dynamicPointInst;
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
