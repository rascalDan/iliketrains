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
#include "maths.h"
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
#include <vector>

ShadowMapper::ShadowMapper(const TextureAbsCoord & s) :
	landmess {shadowLandmass_vert}, dynamicPointInst {shadowDynamicPointInst_vert},
	dynamicPointInstWithTextures {shadowDynamicPointInstWithTextures_vert, shadowDynamicPointInstWithTextures_geom,
			shadowDynamicPointInstWithTextures_frag},
	size {s}
{
	glDebugScope _ {depthMap};
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
	glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, size.x, size.y, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static constexpr RGBA border {std::numeric_limits<RGBA::value_type>::infinity()};
	glTexParameter(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

constexpr auto shadowBands
		= []<GlobalDistance... ints>(const float scaleFactor, std::integer_sequence<GlobalDistance, ints...>) {
			  const auto base = 10'000'000 / pow(scaleFactor, sizeof...(ints) - 1);
			  return std::array {1, static_cast<GlobalDistance>((base * pow(scaleFactor, ints)))...};
		  }(4.6F, std::make_integer_sequence<GlobalDistance, ShadowMapper::SHADOW_BANDS>());

static_assert(shadowBands.front() == 1);
static_assert(shadowBands.back() == 10'000'000);
static_assert(shadowBands.size() == ShadowMapper::SHADOW_BANDS + 1);

std::vector<std::array<RelativePosition3D, 4>>
ShadowMapper::getBandViewExtents(const Camera & camera, const glm::mat4 & lightViewDir)
{
	std::vector<std::array<RelativePosition3D, 4>> bandViewExtents;
	for (const auto dist : shadowBands) {
		const auto extents = camera.extentsAtDist(dist);
		bandViewExtents.emplace_back(extents * [&lightViewDir, cameraPos = camera.getPosition()](const auto & e) {
			return glm::mat3(lightViewDir) * (e.xyz() - cameraPos);
		});
		if (std::none_of(extents.begin(), extents.end(), [targetDist = dist - 1](const auto & e) {
				return e.w > targetDist;
			})) {
			break;
		}
	}
	return bandViewExtents;
}

ShadowMapper::Definitions
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

	const auto lightViewDir = glm::lookAt({}, dir.vector(), up);
	const auto lightViewPoint = camera.getPosition();
	const auto bandViewExtents = getBandViewExtents(camera, lightViewDir);
	Definitions out;
	Sizes sizes;
	using ExtentsBoundingBox = AxisAlignedBoundingBox<RelativeDistance>;
	std::ranges::transform(bandViewExtents | std::views::pairwise, std::back_inserter(out),
			[&lightViewDir, &sizes](const auto & band) mutable {
				const auto & [near, far] = band;
				auto extents = ExtentsBoundingBox::fromPoints(std::span {near.begin(), far.end()});
				extents.min.z -= 10'000.F;
				extents.max.z += 10'000.F;
				const auto lightProjection = glm::ortho(
						extents.min.x, extents.max.x, extents.min.y, extents.max.y, -extents.max.z, -extents.min.z);
				sizes.emplace_back(extents.max - extents.min);
				return lightProjection * lightViewDir;
			});
	for (const auto p : std::initializer_list<const ShadowProgram *> {
				 &landmess, &dynamicPoint, &dynamicPointInst, &dynamicPointInstWithTextures, &stencilShadowProgram}) {
		p->setView(out, sizes, lightViewPoint);
	}
	ExtentsBoundingBox extents {lightViewPoint, lightViewPoint};
	for (const auto & point : bandViewExtents.back()) {
		extents += point;
	}
	const auto lightProjection
			= glm::ortho(extents.min.x, extents.max.x, extents.min.y, extents.max.y, -extents.max.z, -extents.min.z);
	Frustum frustum {lightViewPoint, lightViewDir, lightProjection};
	scene.shadows(*this, frustum);

	glCullFace(GL_BACK);

	return out;
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
