#pragma once

#include "persistence.h"
#include "shape.h"
#include "stdTypeDefs.hpp"
#include "use.h"

class Object : public StdTypeDefs<Object>, public Shape, public Persistence::Persistable {
public:
	Object() = default;
	Object(std::string i);

	CreatedFaces createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const override;

	Use::Collection uses;
	std::string id;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Object>>;
	bool persist(Persistence::PersistenceStore & store) override;
	std::string
	getId() const override
	{
		return id;
	};
};
