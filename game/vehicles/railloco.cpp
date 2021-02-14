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
	auto curLink {linkHist.getAt(0.F)};
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
	location = curLink.first->positionAt(linkDist, curLink.second);
}

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png") { }
