#include "railloco.h"
#include "gfx/gl/transform.h"
#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>
#include <utility>
#include <vector>

void
RailLoco::move(TickDuration dur)
{
	linkDist += dur.count() * speed;
	auto curLink {linkHist.getCurrent()};
	while (linkDist > curLink.first->length) {
		location = curLink.first->positionAt(curLink.first->length, curLink.second);
		const auto & nexts {curLink.first->nexts[1 - curLink.second]};
		const auto next = std::find_if(nexts.begin(), nexts.end(), [ang = location.GetRot().y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].second - ang)) < 0.1F;
		});
		if (next != nexts.end()) {
			linkDist -= curLink.first->length;
			curLink = linkHist.add(next->first, next->second);
		}
		else {
			linkDist = curLink.first->length;
			speed = 0;
		}
	}
}

Transform
RailLoco::getBogeyPosition(float linkDist, float dist) const
{
	float b2linkDist {};
	const auto b2Link = linkHist.getAt(dist - linkDist, &b2linkDist);
	return b2Link.first->positionAt(b2linkDist, b2Link.second);
}

void
RailLoco::updateRailVehiclePosition(RailVehicle * w, float trailBy) const
{
	const auto overhang {(w->length - w->wheelBase) / 2};
	const auto b1Pos = getBogeyPosition(linkDist, trailBy += overhang);
	const auto b2Pos = getBogeyPosition(linkDist, trailBy += wheelBase);
	const auto diff = glm::normalize(b2Pos.GetPos() - b1Pos.GetPos());
	w->location.GetPos() = (b1Pos.GetPos() + b2Pos.GetPos()) / 2.F;
	w->location.GetRot() = {-vector_pitch(diff), vector_yaw(diff), 0};
}

void
RailLoco::tick(TickDuration dur)
{
	move(dur);
	updateRailVehiclePosition(this, 0);
	updateWagons();
}

void
RailLoco::updateWagons() const
{
	// Drag wagons
	float trailBy {length};
	for (const auto & wagon : wagons) {
		const auto w {wagon.lock()};
		updateRailVehiclePosition(w.get(), trailBy);
		trailBy += w->length;
	}
}

void RailWagon::tick(TickDuration) { }

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png")
{
	wheelBase = 15.7F;
	length = 20.F;
	linkDist = wheelBase;
}

Brush47Wagon::Brush47Wagon(const LinkPtr & l) : RailWagon(l, "brush47.obj", "brush47.png")
{
	wheelBase = 15.7F;
	length = 20.F;
	linkDist = wheelBase;
}
