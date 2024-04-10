#pragma once

#include "assetFactory/asset.h"

struct Surface : public Asset {
	friend Persistence::SelectionPtrBase<std::shared_ptr<Surface>>;
	bool persist(Persistence::PersistenceStore & store) override;

	glm::vec3 colorBias;
	float quality;
};
