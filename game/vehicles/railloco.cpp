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
RailLoco::tick(TickDuration dur)
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
	const auto b1pos = curLink.first->positionAt(linkDist, curLink.second);
	const auto b2pos
			= (linkDist >= wheelBase) ? curLink.first->positionAt(linkDist - wheelBase, curLink.second) : [&]() {
				  float b2linkDist {};
				  const auto b2Link = linkHist.getAt(wheelBase - linkDist, &b2linkDist);
				  return b2Link.first->positionAt(b2linkDist, b2Link.second);
			  }();
	location.GetPos() = (b1pos.GetPos() + b2pos.GetPos()) / 2.F;
	const auto diff = glm::normalize(b2pos.GetPos() - b1pos.GetPos());
	location.GetRot().x = -vector_pitch(diff);
	location.GetRot().y = vector_yaw(diff);
	updateWagons();
}

void
RailLoco::updateWagons() const
{
	// Drag wagons
	float trailBy {wheelBase + ((length - wheelBase)) / 2};
	for (const auto & wagon : wagons) {
		const auto w {wagon.lock()};
		auto wTrailBy {trailBy + ((w->length - w->wheelBase) / 2)};
		float b1linkDist {};
		const auto b1Link = linkHist.getAt(wTrailBy - linkDist, &b1linkDist);
		const auto b1Pos = b1Link.first->positionAt(b1linkDist, b1Link.second);
		wTrailBy += w->wheelBase;
		float b2linkDist {};
		const auto b2Link = linkHist.getAt(wTrailBy - linkDist, &b2linkDist);
		const auto b2Pos = b2Link.first->positionAt(b2linkDist, b2Link.second);
		w->location.GetPos() = (b1Pos.GetPos() + b2Pos.GetPos()) / 2.F;
		const auto diff = glm::normalize(b2Pos.GetPos() - b1Pos.GetPos());
		w->location.GetRot().x = -vector_pitch(diff);
		w->location.GetRot().y = vector_yaw(diff);
		trailBy += w->length;
	}
}

void RailWagon::tick(TickDuration) { }

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png")
{
	wheelBase = 15.7F;
	length = 19.38F;
	linkDist = wheelBase;
}

Brush47Wagon::Brush47Wagon(const LinkPtr & l) : RailWagon(l, "brush47.obj", "brush47.png")
{
	wheelBase = 15.7F;
	length = 19.38F;
	linkDist = wheelBase;
}
