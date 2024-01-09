#include "light.h"
#include "location.h"

Light::Light(std::shared_ptr<const Illuminator> type, const Location & position) :
	type {std::move(type)}, location {this->type->instances.acquire(position.getRotationTransform(), position.pos)}
{
}
