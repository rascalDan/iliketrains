#include "train.h"
#include "game/vehicles/linkHistory.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "gfx/renderable.h"
#include "location.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <glm/glm.hpp>
#include <iterator>
#include <maths.h>
#include <random>
#include <utility>

void
Train::move(TickDuration dur)
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
