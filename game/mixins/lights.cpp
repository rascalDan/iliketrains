#include "lights.h"
#include "gfx/renderable.h"

bool
AssetLights::persist(Persistence::PersistenceStore & store)
{
	return STORE_HELPER(pointLight, Persistence::Appender<decltype(pointLight)>)
			&& STORE_HELPER(spotLight, Persistence::Appender<decltype(spotLight)>);
}

void
InstanceLights::lightsEnable(AnyPtr<const AssetLights> asset, uint32_t owner)
{
	auto createLights = [owner](const auto & assetLights, auto & lightInstances, auto commonLights) {
		std::ranges::transform(assetLights | std::views::enumerate, std::inserter(lightInstances, lightInstances.end()),
				[&commonLights, owner](const auto & idxAndLight) {
					const auto & [idx, light] = idxAndLight;
					return std::make_pair(idx, commonLights->acquire(*light, owner));
				});
	};
	createLights(asset->spotLight, spotLightInstances, Renderable::commonSpotLights.lock());
	createLights(asset->pointLight, pointLightInstances, Renderable::commonPointLights.lock());
}

void
InstanceLights::lightsDisable()
{
	spotLightInstances.clear();
	pointLightInstances.clear();
}
