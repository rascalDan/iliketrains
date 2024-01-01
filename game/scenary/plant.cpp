#include "plant.h"
#include "location.h"

Plant::Plant(std::shared_ptr<const Foliage> type, const Location & position) :
	type {std::move(type)}, location {this->type->instances.acquire(position.getRotationTransform(), position.pos)}
{
}
