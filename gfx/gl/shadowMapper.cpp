#include "shadowMapper.h"
#include "collections.h"
#include "game/gamestate.h"
#include "gfx/aabb.h"
#include "gfx/gl/shadowStenciller.h"
#include "gfx/lightDirection.h"
#include "gfx/renderable.h"
#include "gl_traits.h"
#include "gldebug.h"
#include "location.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include <gfx/camera.h>
#include <gfx/gl/shaders/commonShadowPoint-geom.h>
#include <gfx/gl/shaders/shadowDynamicPoint-vert.h>
#include <gfx/gl/shaders/shadowDynamicPointInst-vert.h>
#include <gfx/gl/shaders/shadowDynamicPointInstWithTextures-frag.h>
#include <gfx/gl/shaders/shadowDynamicPointInstWithTextures-geom.h>
#include <gfx/gl/shaders/shadowDynamicPointInstWithTextures-vert.h>
#include <gfx/gl/shaders/shadowDynamicPointStencil-frag.h>
#include <gfx/gl/shaders/shadowDynamicPointStencil-geom.h>
#include <gfx/gl/shaders/shadowDynamicPointStencil-vert.h>
#include <gfx/gl/shaders/shadowLandmass-vert.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <maths.h>

ShadowMapper::ShadowMapper(const TextureAbsCoord & s) :
	landmess {shadowLandmass_vert}, dynamicPointInst {shadowDynamicPointInst_vert},
	dynamicPointInstWithTextures {shadowDynamicPointInstWithTextures_vert, shadowDynamicPointInstWithTextures_geom,
			shadowDynamicPointInstWithTextures_frag},
	size {s}, frustum {{}, {}, {}}
{
	glDebugScope _ {depthMap};
	depthMap.storage(1, GL_DEPTH_COMPONENT16, size || static_cast<GLsizei>(SHADOW_BANDS));
	depthMap.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	depthMap.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	depthMap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	depthMap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static constexpr RGBA border {std::numeric_limits<RGBA::value_type>::infinity()};
	depthMap.parameter(GL_TEXTURE_BORDER_COLOR, border);

	depthMapFBO.texture(GL_DEPTH_ATTACHMENT, depthMap);
	depthMapFBO.drawBuffers(GL_NONE);
	depthMapFBO.assertComplete();
}

constexpr GlobalDistance SHADOW_NEAR = 1;
constexpr GlobalDistance SHADOW_FAR = 10'000'000;
constexpr auto SHADOW_BANDS_DISTS
		= []<GlobalDistance... Ints>(const float scaleFactor, std::integer_sequence<GlobalDistance, Ints...>) {
			  const auto base = SHADOW_FAR / pow(scaleFactor, sizeof...(Ints) - 1);
			  return std::array {SHADOW_NEAR, static_cast<GlobalDistance>((base * pow(scaleFactor, Ints)))...};
		  }(4.6F, std::make_integer_sequence<GlobalDistance, ShadowMapper::SHADOW_BANDS>());

static_assert(SHADOW_BANDS_DISTS.front() == 1);
static_assert(SHADOW_BANDS_DISTS.back() == SHADOW_FAR);
static_assert(SHADOW_BANDS_DISTS.size() == ShadowMapper::SHADOW_BANDS + 1);

size_t
ShadowMapper::getBandViewExtents(
		BandViewExtents & bandViewExtents, const Camera & camera, const glm::mat4 & lightViewDir)
{
	size_t band = 0;
	for (const auto dist : SHADOW_BANDS_DISTS) {
		const auto extents = camera.extentsAtDist(dist);
		bandViewExtents[band++] = extents * [&lightViewDir, cameraPos = camera.getPosition()](const auto & extent) {
			return glm::mat3(lightViewDir) * (extent.xyz() - cameraPos);
		};
		if (std::ranges::none_of(extents, [dist](const auto & extent) {
				return extent.w >= dist;
			})) {
			break;
		}
	}
	return band;
}

const Frustum &
ShadowMapper::preFrame(const LightDirection & dir, const Camera & camera)
{
	const auto lightViewDir = glm::lookAt({}, dir.vector(), Camera::upFromForward(dir.vector()));
	const auto lightViewPoint = camera.getPosition();
	const auto bandViewExtentCount = getBandViewExtents(bandViewExtents, camera, lightViewDir);
	const auto activeBandViewExtents = std::span(bandViewExtents).subspan(0, bandViewExtentCount);

	using ExtentsBoundingBox = AxisAlignedBoundingBox<RelativeDistance>;
	for (auto out = std::make_pair(sizes.begin(), definitions.begin());
			const auto & [near, far] : activeBandViewExtents | std::views::pairwise) {
		const auto extents = ExtentsBoundingBox::fromPoints(std::span {near.begin(), far.end()});
		const auto lightProjection = glm::ortho(
				extents.min.x, extents.max.x, extents.min.y, extents.max.y, -extents.max.z, -extents.min.z);
		*out.first++ = extents.max - extents.min;
		*out.second++ = lightProjection * lightViewDir;
	}

	const auto extents = ExtentsBoundingBox::fromPoints(activeBandViewExtents.back()) += {};
	const auto lightProjection
			= glm::ortho(extents.min.x, extents.max.x, extents.min.y, extents.max.y, -extents.max.z, -extents.min.z);
	frustum = {lightViewPoint, lightViewDir, lightProjection};
	return frustum;
}

std::span<const glm::mat4>
ShadowMapper::update(const SceneProvider & scene, const LightDirection & dir, const Camera & camera) const
{
	glDebugScope _ {depthMap};
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);

	shadowStenciller.setLightDirection(dir);
	for (const auto & [id, asset] : gameState->assets) {
		if (const auto r = asset.getAs<const Renderable>()) {
			r->updateStencil(shadowStenciller);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, size.x, size.y);

	const auto lightViewPoint = camera.getPosition();
	for (const auto p : std::initializer_list<const ShadowProgram *> {
				 &landmess, &dynamicPoint, &dynamicPointInst, &dynamicPointInstWithTextures, &stencilShadowProgram}) {
		p->setView(definitions, sizes, lightViewPoint);
	}
	scene.shadows(*this, frustum);

	glCullFace(GL_BACK);

	return definitions;
}

ShadowMapper::ShadowProgram::ShadowProgram(const Shader & vs) : Program {vs, commonShadowPoint_geom} { }

ShadowMapper::ShadowProgram::ShadowProgram(const Shader & vs, const Shader & gs, const Shader & fs) :
	Program {vs, gs, fs}
{
}

void
ShadowMapper::ShadowProgram::setView(const std::span<const glm::mat4x4> viewProjection,
		const std::span<const RelativePosition3D> sizes, const GlobalPosition3D viewPoint) const
{
	use();
	glUniform(viewPointLoc, viewPoint);
	glUniform(viewProjectionLoc, viewProjection);
	if (sizesLoc) {
		glUniform(sizesLoc, sizes);
	}
	glUniform(viewProjectionsLoc, static_cast<GLint>(viewProjection.size()));
}

void
ShadowMapper::ShadowProgram::use() const
{
	glUseProgram(*this);
}

ShadowMapper::DynamicPoint::DynamicPoint() : ShadowProgram {shadowDynamicPoint_vert} { }

void
ShadowMapper::DynamicPoint::use(const Location & location) const
{
	glUseProgram(*this);
	setModel(location);
}

void
ShadowMapper::DynamicPoint::setModel(const Location & location) const
{
	glUniform(modelLoc, location.getRotationTransform());
	glUniform(modelPosLoc, location.pos);
}

ShadowMapper::StencilShadowProgram::StencilShadowProgram() :
	ShadowProgram {shadowDynamicPointStencil_vert, shadowDynamicPointStencil_geom, shadowDynamicPointStencil_frag}
{
}

void
ShadowMapper::StencilShadowProgram::use(const RelativePosition3D & centre, const float size) const
{
	Program::use();
	glUniform(centreLoc, centre);
	glUniform(sizeLoc, size);
}
