#include "aabb.h"
#include <algorithm>
#include <tuple>

AxisAlignedBoundingBox
AxisAlignedBoundingBox::fromPoints(const std::span<const GlobalPosition3D> points)
{
	using Limits = std::numeric_limits<GlobalDistance>;
	static constexpr const auto INITIAL
			= std::make_pair(GlobalPosition3D {Limits::max()}, GlobalPosition3D {Limits::min()});
	return std::make_from_tuple<AxisAlignedBoundingBox>(
			std::ranges::fold_left(points, INITIAL, [](const auto & prev, const auto & point) {
				auto & [min, max] = prev;
				return std::make_pair(glm::min(min, point), glm::max(max, point));
			}));
}

AxisAlignedBoundingBox &
AxisAlignedBoundingBox::operator+=(const GlobalPosition3D & point)
{
	min = glm::min(min, point);
	max = glm::max(max, point);
	return *this;
}
