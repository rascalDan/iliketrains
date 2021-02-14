#include "vehicle.h"
#include "game/network/link.h"
#include <array>
#include <memory>
#include <utility>

Vehicle::Vehicle(const LinkPtr & l, const std::string & obj, const std::string & tex) :
	Physical(l->ends.front().first->pos, obj, tex), link(l)
{
}
