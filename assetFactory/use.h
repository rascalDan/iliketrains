#pragma once

#include "faceController.h"
#include "modelFactoryMesh_fwd.h"
#include "persistence.h"
#include "shape.h"
#include "stdTypeDefs.hpp"
#include "style.h"

class Use : public StdTypeDefs<Use>, public Mutation, public Style, public Persistence::Persistable {
public:
	using FaceControllers = std::map<std::string, std::unique_ptr<FaceController>>;

	Shape::CreatedFaces createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const;

	Shape::CPtr type;
	FaceControllers faceControllers;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Use>, true>;
	bool persist(Persistence::PersistenceStore & store) override;
	std::string
	getId() const override
	{
		return {};
	};
};
