#include "bufferedLocation.h"
#include "location.h"
#include "maths.h"
#include <glm/gtx/transform.hpp>

BufferedLocation::BufferedLocation(glm::vec3 p, glm::vec3 r) : BufferedLocation {Location {p, r}} { }

BufferedLocation::BufferedLocation(const Location & l) : loc {l} { }

BufferedLocation::operator const Location &() const
{
	return loc;
}

BufferedLocation &
BufferedLocation::operator=(const Location & l)
{
	loc = l;
	updateBuffer();
	return *this;
}

glm::vec3
BufferedLocation::position() const
{
	return loc.pos;
}

glm::vec3
BufferedLocation::rotation() const
{
	return loc.rot;
}

void
BufferedLocation::setPosition(glm::vec3 p, bool update)
{
	loc.pos = p;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setRotation(glm::vec3 r, bool update)
{
	loc.rot = r;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setLocation(glm::vec3 p, glm::vec3 r)
{
	loc.pos = p;
	loc.rot = r;
	updateBuffer();
}

glm::mat4
BufferedLocation::getTransform() const
{
	return loc.getTransform();
}
