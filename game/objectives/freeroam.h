#ifndef FREEROAM_H
#define FREEROAM_H

#include "game/activities/go.h"
#include "game/objective.h"

class FreeRoam : public Objective {
public:
	using Objective::Objective;

	[[nodiscard]] ActivityPtr
	createActivity() const override
	{
		return std::make_unique<Go>();
	}
};

#endif
