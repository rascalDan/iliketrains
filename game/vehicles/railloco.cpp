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
	while (linkDist > link->length) {
		location = link->positionAt(link->length, linkDir);
		const auto & nexts {link->nexts[1 - linkDir]};
		const auto next = std::find_if(nexts.begin(), nexts.end(), [ang = location.GetRot().y](const Link::Next & n) {
			return std::abs(normalize(n.first->ends[n.second].second - ang)) < 0.1F;
		});
		if (next != nexts.end()) {
			linkDist -= link->length;
			link = next->first;
			linkDir = next->second;
		}
		else {
			linkDist = link->length;
			speed = 0;
		}
	}
	location = link->positionAt(linkDist, linkDir);
}

Brush47::Brush47(const LinkPtr & l) : RailLoco(l, "brush47.obj", "brush47.png") { }
