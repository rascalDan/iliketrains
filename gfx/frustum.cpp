#include "frustum.h"
#include "aabb.h"
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

bool
Frustum::contains(const AxisAlignedBoundingBox & aabb) const
{
	static constexpr auto EXTENT_CORNER_IDXS = [] {
		using Extent = GlobalPosition3D AxisAlignedBoundingBox::*;
		constexpr auto EXTENTS = std::array {&AxisAlignedBoundingBox::min, &AxisAlignedBoundingBox::max};
		std::array<glm::vec<3, Extent>, 2ZU * 2ZU * 2ZU> out {};
		std::ranges::copy(std::views::cartesian_product(EXTENTS, EXTENTS, EXTENTS)
						| std::views::transform(
								std::make_from_tuple<glm::vec<3, Extent>, std::tuple<Extent, Extent, Extent>>),
				out.begin());
		return out;
	}();

	const std::array<RelativePosition4D, 8> corners
			= EXTENT_CORNER_IDXS * [relativeAabb = aabb - position](auto idxs) -> glm::vec4 {
		return {(relativeAabb.*(idxs.x)).x, (relativeAabb.*(idxs.y)).y, (relativeAabb.*(idxs.z)).z, 1.F};
	};
	return std::ranges::none_of(planes, [&corners](const auto & frustumPlane) {
		return (std::ranges::all_of(corners, [&frustumPlane](const auto & corner) {
			return glm::dot(frustumPlane, corner) < 0.F;
		}));
	});
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
