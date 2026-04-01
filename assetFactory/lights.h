#pragma once

#include "gfx/models/lights.h"
#include "persistence.h"
#include "stdTypeDefs.h"

struct SpotLight : Persistence::Persistable, SpotLightDef, StdTypeDefs<SpotLight> {
private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<SpotLight>>;
	bool persist(Persistence::PersistenceStore & store) override;
};

struct PointLight : Persistence::Persistable, PointLightDef, StdTypeDefs<PointLight> {
private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<PointLight>>;
	bool persist(Persistence::PersistenceStore & store) override;
};

