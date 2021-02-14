#include "vehicle.h"

Vehicle::Vehicle(LinkPtr l, const std::string & obj, const std::string & tex) :
	Physical(l->ends.front().first->pos, obj, tex), link(l)
{
}
