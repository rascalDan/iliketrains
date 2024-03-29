#include "bufferedLocation.h"
#include "location.h"
#include <glm/gtx/transform.hpp>

BufferedLocation::BufferedLocation(GlobalPosition3D p, Rotation3D r) : BufferedLocation {Location {p, r}} { }

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

GlobalPosition3D
BufferedLocation::position() const
{
	return loc.pos;
}

Rotation3D
BufferedLocation::rotation() const
{
	return loc.rot;
}

void
BufferedLocation::setPosition(GlobalPosition3D p, bool update)
{
	loc.pos = p;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setRotation(Rotation3D r, bool update)
{
	loc.rot = r;
	if (update) {
		updateBuffer();
	}
}

void
BufferedLocation::setLocation(GlobalPosition3D p, Rotation3D r)
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
