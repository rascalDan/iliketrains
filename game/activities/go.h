#ifndef GO_H
#define GO_H

#include "../activity.h"
#include "../activityOf.h"
#include <optional>

class Go : public Activity::Of<Go> {
public:
	Go() = default;
	explicit Go(float dist);

	std::optional<float> dist;
};

#endif
