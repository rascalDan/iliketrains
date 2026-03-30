#include "light.h"
#include "location.h"

Light::Light(std::shared_ptr<const Illuminator> type, const Location & position) :
	type {std::move(type)},
	instance {this->type->instances.acquire(Renderable::commonLocationData.lock()->acquire(position))}
{
}
