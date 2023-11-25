#include "bufferedLocation.h"
#include "location.h"
#include <glm/gtx/transform.hpp>

BufferedLocation::BufferedLocation(Position3D p, Rotation3D r) : BufferedLocation {Location {p, r}} { }

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

Position3D
BufferedLocation::position() const
{
	return loc.pos;
}

Position3D
BufferedLocation::rotation() const
{
	return loc.rot;
}

void
BufferedLocation::setPosition(Position3D p, bool update)
{
	loc.pos = p;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setRotation(Position3D r, bool update)
{
	loc.rot = r;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setLocation(Position3D p, Rotation3D r)
{
	loc.pos = p;
	loc.rot = r;
	updateBuffer();
}

glm::mat4
BufferedLocation::getRotationTransform() const
{
	return loc.getRotationTransform();
}

void
BufferedLocationUpdater::updateBuffer() const
{
	onUpdate(this);
}
