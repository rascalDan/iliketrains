#pragma once

#include "config/types.h"
#include "gfx/frustum.h"
#include "gfx/gl/shadowStenciller.h"
#include "lib/glArrays.h"
#include "program.h"
#include <array>
#include <gfx/models/texture.h>
#include <glm/vec2.hpp>
#include <span>

class SceneProvider;
class Camera;
class LightDirection;

class ShadowMapper {
public:
	explicit ShadowMapper(const TextureAbsCoord & size);

	static constexpr std::size_t SHADOW_BANDS {4};

	using Definitions = std::array<glm::mat4, SHADOW_BANDS>;
	using Sizes = std::array<RelativePosition3D, SHADOW_BANDS>;

	const Frustum & preFrame(const LightDirection & direction, const Camera &);
	[[nodiscard]] std::span<const glm::mat4> update(
			const SceneProvider &, const LightDirection & direction, const Camera &) const;

	class ShadowProgram : public Program {
	public:
		explicit ShadowProgram(const Shader & vs);
		explicit ShadowProgram(const Shader & vs, const Shader & gs, const Shader & fs);

		void setView(const std::span<const glm::mat4x4>, const std::span<const RelativePosition3D>,
				const GlobalPosition3D) const;
		void use() const;

	private:
		RequiredUniformLocation viewProjectionLoc {*this, "viewProjection"};
		RequiredUniformLocation viewProjectionsLoc {*this, "viewProjections"};
		UniformLocation sizesLoc {*this, "sizes"};
		RequiredUniformLocation viewPointLoc {*this, "viewPoint"};
	};

	class DynamicPoint : public ShadowProgram {
	public:
		DynamicPoint();
		void use(const Location &) const;
		void setModel(const Location &) const;

	private:
		RequiredUniformLocation modelLoc {*this, "model"};
		RequiredUniformLocation modelPosLoc {*this, "modelPos"};
	};

	class StencilShadowProgram : public ShadowProgram {
	public:
		StencilShadowProgram();
		void use(const RelativePosition3D & centre, const float size) const;

	private:
		RequiredUniformLocation centreLoc {*this, "centre"};
		RequiredUniformLocation sizeLoc {*this, "size"};
	};

	ShadowProgram landmess, dynamicPointInst, dynamicPointInstWithTextures;
	DynamicPoint dynamicPoint;
	StencilShadowProgram stencilShadowProgram;

	void
	bind(GLenum unit) const
	{
		depthMap.bind(GL_TEXTURE_2D_ARRAY, unit);
	}

private:
	using BandViewExtents = std::array<std::array<RelativePosition3D, 4>, SHADOW_BANDS + 1>;
	[[nodiscard]] static size_t getBandViewExtents(BandViewExtents &, const Camera &, const glm::mat4 & lightView);
	glFrameBuffer depthMapFBO;
	glTexture depthMap;
	TextureAbsCoord size;

	BandViewExtents bandViewExtents;
	Definitions definitions;
	Sizes sizes;
	Frustum frustum;

	mutable ShadowStenciller shadowStenciller;
};
