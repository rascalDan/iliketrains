#pragma once

#include "config/types.h"
#include "gfx/models/mesh.h"
#include "stdTypeDefs.h"
#include "use.h"

class FactoryMesh : public Persistence::Persistable, public StdTypeDefs<FactoryMesh> {
public:
	Mesh::Ptr createMesh() const;

	std::string id;
	Size3D size;
	Use::Collection uses;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<FactoryMesh>>;
	bool persist(Persistence::PersistenceStore & store) override;
};
