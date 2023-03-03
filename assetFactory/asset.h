#pragma once

#include "factoryMesh.h"
#include "persistence.h"
#include <stdTypeDefs.hpp>

class Asset : public Persistence::Persistable, public StdTypeDefs<Asset> {
public:
	std::string id;
	std::string name;

	FactoryMesh::Collection meshes;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Asset>>;
	bool persist(Persistence::PersistenceStore & store) override;
};
