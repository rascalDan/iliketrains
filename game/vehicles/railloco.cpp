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
}

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png")
{
	wheelBase = 15.7F;
	linkDist = wheelBase;
}
