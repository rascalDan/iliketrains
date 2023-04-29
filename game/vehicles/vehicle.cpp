#include "vehicle.h"
#include "game/activities/idle.h"
#include "game/activity.h"
#include "game/network/link.h"
#include "game/objective.h"
#include "game/orders.h"
#include "game/vehicles/linkHistory.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <iterator>
#include <location.h>
#include <maths.h>
#include <utility>
#include <vector>

Vehicle::Vehicle(const Link::Ptr & l, float ld) : linkDist {ld}
{
	linkHist.add(l, 0);
	currentActivity = std::make_unique<Idle>();
}

void
Vehicle::move(TickDuration dur)
{
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		const auto location = curLink.first->positionAt(curLink.first->length, curLink.second);
		auto nexts {curLink.first->ends[1 - curLink.second].nexts};
		auto last = std::remove_if(nexts.begin(), nexts.end(), [ang = location.rot.y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].dir - ang)) > 0.1F;
		});
		if (last != nexts.begin()) {
			auto next = (std::distance(nexts.begin(), last) > 1) ? orders.current()->navigate(nexts.cbegin(), last)
																 : nexts.front();
			linkDist -= curLink.first->length;
			curLink = linkHist.add(next.first, next.second);
		}
		else {
			linkDist = curLink.first->length;
			speed = 0;
		}
	}
}
