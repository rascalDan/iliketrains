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
	auto curLink {link.lock()};
	while (linkDist > curLink->length) {
		location = curLink->positionAt(curLink->length, linkDir);
		const auto & nexts {curLink->nexts[1 - linkDir]};
		const auto next = std::find_if(nexts.begin(), nexts.end(), [ang = location.GetRot().y](const Link::Next & n) {
			return std::abs(normalize(n.first.lock()->ends[n.second].second - ang)) < 0.1F;
		});
		if (next != nexts.end()) {
			linkDist -= curLink->length;
			link = next->first;
			curLink = link.lock();
			linkDir = next->second;
		}
		else {
			linkDist = curLink->length;
			speed = 0;
		}
	}
	location = curLink->positionAt(linkDist, linkDir);
}

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png") { }
