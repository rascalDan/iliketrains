#pragma once

#include "maths.h"
#include <algorithm>
#include <tuple>

template<Arithmetic T, glm::qualifier Q = glm::defaultp> class AxisAlignedBoundingBox {
public:
	using V = glm::vec<3, T, Q>;
	AxisAlignedBoundingBox() = default;

	AxisAlignedBoundingBox(const V & min, const V & max) : min {min}, max {max} { }

	AxisAlignedBoundingBox &
	operator+=(const V & point)
	{
		min = glm::min(min, point);
		max = glm::max(max, point);
		return *this;
	}

	AxisAlignedBoundingBox
	operator-(const V & viewPoint) const
	{
		return {min - viewPoint, max - viewPoint};
	}

	[[nodiscard]] static AxisAlignedBoundingBox
	fromPoints(auto && points)
	{
		using Limits = std::numeric_limits<T>;
		static constexpr const auto INITIAL = std::make_pair(V {Limits::max()}, V {Limits::min()});
		return std::make_from_tuple<AxisAlignedBoundingBox<T, Q>>(
				std::ranges::fold_left(points, INITIAL, [](const auto & prev, const auto & point) {
					auto & [min, max] = prev;
					return std::make_pair(glm::min(min, point), glm::max(max, point));
				}));
	}

	V min, max;
};
