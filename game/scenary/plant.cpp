#include "plant.h"
#include "location.hpp"

Plant::Plant(std::shared_ptr<const Foliage> type, const Location & position) :
	type {std::move(type)}, location {this->type->instances.acquire(position.getTransform())}
{
}
