#pragma once

#include <game/activity.h>
#include <game/network/link.h>
#include <game/objective.h>

class FreeRoam : public Objective {
public:
	using Objective::Objective;

	[[nodiscard]] ActivityPtr createActivity() const override;
	[[nodiscard]] Link::Next navigate(Link::Nexts::const_iterator, Link::Nexts::const_iterator) const override;
};
