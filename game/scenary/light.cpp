#include "light.h"
#include "location.h"

Light::Light(std::shared_ptr<const Illuminator> type, const Location & position) :
	type {std::move(type)},
	instance {this->type->instances.acquire(Renderable::commonLocationData.lock()->acquire(position))}
{
	std::ranges::transform(this->type->spotLight, std::back_inserter(spotLightInstances),
			[spotLights = Renderable::commonSpotLights.lock(), this](const auto & spotLight) {
				return spotLights->acquire(*spotLight, instance->location.index);
			});
	std::ranges::transform(this->type->pointLight, std::back_inserter(pointLightInstances),
			[pointLights = Renderable::commonPointLights.lock(), this](const auto & pointLight) {
				return pointLights->acquire(*pointLight, instance->location.index);
			});
}
