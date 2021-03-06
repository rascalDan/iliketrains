#include "vehicle.h"
#include "game/network/link.h"
#include "game/vehicles/linkHistory.h"

Vehicle::Vehicle(const LinkPtr & l, float ld) : linkDist {ld}
{
	linkHist.add(l, 0);
}
