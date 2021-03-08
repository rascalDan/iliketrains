#include "vehicle.h"
#include "game/activity.h"
#include "game/network/link.h"
#include "game/objective.h"
#include "game/objectives/freeroam.h"
#include "game/orders.h"
#include "game/vehicles/linkHistory.h"

Vehicle::Vehicle(const LinkPtr & l, float ld) : linkDist {ld}
{
	linkHist.add(l, 0);
	orders.create<FreeRoam>(&orders);
	currentActivity = orders.current()->createActivity();
}
