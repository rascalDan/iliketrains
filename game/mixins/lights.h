#pragma once

#include "assetFactory/lights.h"
#include "gfx/gl/instanceVertices.h"
#include <flat_map>

class AssetLights {
protected:
	bool persist(Persistence::PersistenceStore & store);
	template<typename T> using LightVec = std::vector<typename T::Ptr>;

	LightVec<SpotLight> spotLight;
	LightVec<PointLight> pointLight;

	friend class InstanceLights;
};

class InstanceLights {
protected:
	template<typename V> using LightInstanceMap = std::flat_map<size_t, typename InstanceVertices<V>::InstanceProxy>;
	LightInstanceMap<SpotLightVertex> spotLightInstances;
	LightInstanceMap<PointLightVertex> pointLightInstances;

	void lightsEnable(AnyPtr<const AssetLights>, uint32_t);
	void lightsDisable();
};
