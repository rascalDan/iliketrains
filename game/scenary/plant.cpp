#include "plant.h"

Plant::Plant(std::shared_ptr<const Foliage> type, Location position) :
	type {std::move(type)},
	location {this->type->instances.acquire(glm::translate(position.pos) * rotate_ypr(position.rot))}
{
}
