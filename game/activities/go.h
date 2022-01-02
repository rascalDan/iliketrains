#pragma once

#include "../activity.h"
#include "../activityOf.h"
#include <optional>

class Go : public Activity::Of<Go> {
public:
	Go() = default;
	explicit Go(float dist);

	std::optional<float> dist;
};
