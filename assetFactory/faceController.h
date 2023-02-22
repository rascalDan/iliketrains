#pragma once

#include "modelFactoryMesh_fwd.h"
#include "mutation.h"
#include "persistence.h"
#include "shape.h"
#include <map>
#include <string>

class FaceController : public Mutation, public Persistence::Persistable {
public:
	using FaceControllers = std::map<std::string, std::unique_ptr<FaceController>>;

	void apply(ModelFactoryMesh & mesh, const std::string & name, Shape::CreatedFaces & faces) const;

	std::string id;
	std::string colour;
	std::string type;
	bool smooth {false};
	FaceControllers faceControllers;

private:
	friend Persistence::SelectionPtrBase<std::unique_ptr<FaceController>, false>;
	bool persist(Persistence::PersistenceStore & store) override;
	std::string
	getId() const override
	{
		return {};
	};
};
