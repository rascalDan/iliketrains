#include "train.h"
#include "game/vehicles/linkHistory.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "gfx/renderable.h"
#include "location.hpp"
#include <algorithm>
#include <functional>
#include <utility>

void
Train::render(const Shader & shader) const
{
	apply(&Renderable::render, shader);
}

Location
Train::getBogiePosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

void
Train::tick(TickDuration dur)
{
	currentActivity->apply(this, dur);
	move(dur);

	float trailBy {0.F};
	apply(&RailVehicle::move, this, std::ref(trailBy));
}

void
Train::doActivity(const Go *, TickDuration dur)
{
	const auto maxSpeed = objects.front()->rvClass->maxSpeed;
	if (speed != maxSpeed) {
		speed += ((maxSpeed - speed) * dur.count());
	}
}

void
Train::doActivity(const Idle *, TickDuration dur)
{
	if (speed != 0.F) {
		speed -= std::min(speed, 30.F * dur.count());
	}
}
