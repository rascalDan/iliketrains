#include "shadowMapper.h"
#include "camera.h"
#include "collections.h"
#include "game/gamestate.h"
#include "gfx/gl/shaders/fs-shadowDynamicPointInstWithTextures.h"
#include "gfx/gl/shaders/fs-shadowDynamicPointStencil.h"
#include "gfx/gl/shaders/gs-commonShadowPoint.h"
#include "gfx/gl/shaders/gs-shadowDynamicPointInstWithTextures.h"
#include "gfx/gl/shaders/gs-shadowDynamicPointStencil.h"
#include "gfx/gl/shaders/vs-shadowDynamicPoint.h"
#include "gfx/gl/shaders/vs-shadowDynamicPointInst.h"
#include "gfx/gl/shaders/vs-shadowDynamicPointInstWithTextures.h"
#include "gfx/gl/shaders/vs-shadowDynamicPointStencil.h"
#include "gfx/gl/shaders/vs-shadowLandmass.h"
#include "gfx/gl/shadowStenciller.h"
#include "gfx/lightDirection.h"
#include "gfx/renderable.h"
#include "gl_traits.h"
#include "location.h"
#include "maths.h"
#include "sceneProvider.h"
#include "sceneShader.h"
#include "sorting.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <vector>

ShadowMapper::ShadowMapper(const TextureAbsCoord & s) :
	landmess {shadowLandmass_vs}, dynamicPointInst {shadowDynamicPointInst_vs},
	dynamicPointInstWithTextures {shadowDynamicPointInstWithTextures_vs, shadowDynamicPointInstWithTextures_gs,
			shadowDynamicPointInstWithTextures_fs},
	size {s}
{
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
		  }(6.6F, std::make_integer_sequence<GlobalDistance, ShadowMapper::SHADOW_BANDS>());

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
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);

	shadowStenciller.setLightDirection(dir);
	for (const auto & [id, asset] : gameState->assets) {
		if (const auto r = std::dynamic_pointer_cast<const Renderable>(asset)) {
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
	std::transform(bandViewExtents.begin(), std::prev(bandViewExtents.end()), std::next(bandViewExtents.begin()),
			std::back_inserter(out),
			[bands = bandViewExtents.size() - 2, &lightViewDir, &sizes](const auto & near, const auto & far) mutable {
				const auto extents_minmax
						= [extents = std::span {near.begin(), far.end()}](auto && comp, RelativeDistance extra) {
							  const auto mm = std::minmax_element(extents.begin(), extents.end(), comp);
							  return std::make_pair(comp.get(*mm.first) - extra, comp.get(*mm.second) + extra);
						  };
				const std::array extents = {extents_minmax(CompareBy {0}, 0), extents_minmax(CompareBy {1}, 0),
						extents_minmax(CompareBy {2}, 10'000)};

				const auto lightProjection = [](const auto & x, const auto & y, const auto & z) {
					return glm::ortho(x.first, x.second, y.first, y.second, -z.second, -z.first);
				}(extents[0], extents[1], extents[2]);

				sizes.emplace_back(extents[0].second - extents[0].first, extents[1].second - extents[1].first,
						extents[2].second - extents[2].first);
				return lightProjection * lightViewDir;
			});
	for (const auto p : std::initializer_list<const ShadowProgram *> {
				 &landmess, &dynamicPoint, &dynamicPointInst, &dynamicPointInstWithTextures, &stencilShadowProgram}) {
		p->setView(out, sizes, lightViewPoint);
	}
	scene.shadows(*this);

	glCullFace(GL_BACK);

	return out;
}

ShadowMapper::ShadowProgram::ShadowProgram(const Shader & vs) : Program {vs, commonShadowPoint_gs} { }

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

ShadowMapper::DynamicPoint::DynamicPoint() : ShadowProgram {shadowDynamicPoint_vs} { }

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
	ShadowProgram {shadowDynamicPointStencil_vs, shadowDynamicPointStencil_gs, shadowDynamicPointStencil_fs}
{
}

void
ShadowMapper::StencilShadowProgram::use(const RelativePosition3D & centre, const float size) const
{
	Program::use();
	glUniform(centreLoc, centre);
	glUniform(sizeLoc, size);
}
