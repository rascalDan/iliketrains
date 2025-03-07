#pragma once

#include "config/types.h"
#include <span>

class AxisAlignedBoundingBox {
public:
	AxisAlignedBoundingBox() = default;

	AxisAlignedBoundingBox(const GlobalPosition3D & min, const GlobalPosition3D & max) : min {min}, max {max} { }

	AxisAlignedBoundingBox & operator+=(const GlobalPosition3D & point);

	AxisAlignedBoundingBox operator-(const GlobalPosition3D & viewPoint) const;

	[[nodiscard]] static AxisAlignedBoundingBox fromPoints(std::span<const GlobalPosition3D> points);

	GlobalPosition3D min, max;
};
