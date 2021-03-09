#include "vehicle.h"
#include "game/activity.h"
#include "game/network/link.h"
#include "game/objective.h"
#include "game/objectives/freeroam.h"
#include "game/orders.h"
#include "game/vehicles/linkHistory.h"
#include <algorithm>
#include <array>
#include <game/worldobject.h>
#include <glm/glm.hpp>
#include <iterator>
#include <location.hpp>
#include <maths.h>
#include <random>
#include <utility>
#include <vector>

Vehicle::Vehicle(const LinkPtr & l, float ld) : linkDist {ld}
{
	linkHist.add(l, 0);
	orders.create<FreeRoam>(&orders);
	currentActivity = orders.current()->createActivity();
}

void
Vehicle::move(TickDuration dur)
{
	static std::mt19937 gen(std::random_device {}());
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		const auto location = curLink.first->positionAt(curLink.first->length, curLink.second);
		auto nexts {curLink.first->nexts[1 - curLink.second]};
		auto last = std::remove_if(nexts.begin(), nexts.end(), [ang = location.rot.y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].second - ang)) > 0.1F;
		});
		if (last != nexts.begin()) {
			auto off = std::uniform_int_distribution<>(0, std::distance(nexts.begin(), last) - 1)(gen);
			linkDist -= curLink.first->length;
			curLink = linkHist.add(nexts[off].first, nexts[off].second);
		}
		else {
			linkDist = curLink.first->length;
			speed = 0;
		}
	}
}
