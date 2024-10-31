#include "train.h"
#include "game/vehicles/linkHistory.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "location.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <utility>

template<typename> class Ray;

constexpr auto DECELERATION_RATE = 60000.F;
constexpr auto ACCELERATIONS_RATE = 30000.F;

Location
Train::getBogiePosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

bool
Train::intersectRay(const Ray<GlobalPosition3D> & ray, BaryPosition & baryPos, RelativeDistance & distance) const
{
	return applyOne(&RailVehicle::intersectRay, ray, baryPos, distance) != end();
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
Train::doActivity(Go * go, TickDuration dur)
{
	const auto maxSpeed = objects.front()->rvClass->maxSpeed;
	if (go->dist) {
		*go->dist -= speed * dur.count();
		if (*go->dist < (speed * speed) / DECELERATION_RATE) {
			speed -= std::min(speed, ACCELERATIONS_RATE * dur.count());
		}
		else {
			if (speed != maxSpeed) {
				speed += ((maxSpeed - speed) * dur.count());
			}
		}
	}
	else {
		if (speed != maxSpeed) {
			speed += ((maxSpeed - speed) * dur.count());
		}
	}
}

void
Train::doActivity(Idle *, TickDuration dur)
{
	if (speed != 0.F) {
		speed -= std::min(speed, DECELERATION_RATE * dur.count());
	}
}
