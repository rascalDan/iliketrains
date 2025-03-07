#pragma once

#include "config/types.h"
#include <algorithm>
#include <tuple>

class AxisAlignedBoundingBox {
public:
	AxisAlignedBoundingBox() = default;

	AxisAlignedBoundingBox(const GlobalPosition3D & min, const GlobalPosition3D & max) : min {min}, max {max} { }

	AxisAlignedBoundingBox & operator+=(const GlobalPosition3D & point);

	AxisAlignedBoundingBox operator-(const GlobalPosition3D & viewPoint) const;

	[[nodiscard]] static AxisAlignedBoundingBox
	fromPoints(auto && points)
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

	GlobalPosition3D min, max;
};
