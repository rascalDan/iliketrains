#pragma once

#include "gfx/models/mesh.h"
#include "stdTypeDefs.hpp"
#include "use.h"

class FactoryMesh : public Persistence::Persistable, public StdTypeDefs<FactoryMesh> {
public:
	Mesh::Ptr createMesh() const;

	std::string id;
	glm::vec3 size;
	Use::Collection uses;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<FactoryMesh>>;
	bool persist(Persistence::PersistenceStore & store) override;
};
