#include "frustum.h"
#include <algorithm>
#include <collections.h>
#include <glm/ext/matrix_transform.hpp>

static constexpr auto PLANES = std::array {0, 1, 2} * std::array {1.F, -1.F};

Frustum::Frustum(const GlobalPosition3D & pos, const glm::mat4 & view, const glm::mat4 & projection) :
	position {pos}, view {view}, projection {projection}, viewProjection {}, inverseViewProjection {}, planes {}
{
	updateCache();
}

void
Frustum::updateView(const glm::mat4 & newView)
{
	view = newView;
	updateCache();
}

void
Frustum::updateCache()
{
	viewProjection = projection * view;
	inverseViewProjection = glm::inverse(viewProjection);
	std::ranges::transform(PLANES, planes.begin(), [vpt = glm::transpose(viewProjection)](const auto & idxs) {
		const auto [idx, sgn] = idxs;
		return vpt[3] + (vpt[idx] * sgn);
	});
}
