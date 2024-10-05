#pragma once

#include "config/types.h"

class LightDirection {
public:
	// NOLINTNEXTLINE(hicpp-explicit-conversions) deliberately a helper
	LightDirection(Direction2D sunPos);

	[[nodiscard]] Direction2D
	position() const noexcept
	{
		return pos;
	}

	[[nodiscard]] Direction3D
	vector() const noexcept
	{
		return vec;
	}

	[[nodiscard]] float
	vertical() const noexcept
	{
		return vert;
	}

private:
	Direction2D pos;
	Direction3D vec;
	float vert;
};
