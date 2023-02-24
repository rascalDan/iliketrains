#pragma once

#include "modelFactoryMesh_fwd.h"
#include "mutation.h"
#include "persistence.h"
#include "shape.h"
#include "style.h"
#include <map>
#include <string>

class FaceController : public Mutation, public Style, public Persistence::Persistable {
public:
	using FaceControllers = std::map<std::string, std::unique_ptr<FaceController>>;

	void apply(ModelFactoryMesh & mesh, const Style::StyleStack & parents, const std::string & name,
			Shape::CreatedFaces & faces) const;

	std::string id;
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
