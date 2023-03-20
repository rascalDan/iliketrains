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

	void apply(ModelFactoryMesh & mesh, const Style::StyleStack & parents, const std::string & names,
			Shape::CreatedFaces & faces) const;

	std::string id;
	std::string type;
	bool smooth {false};
	FaceControllers faceControllers;

private:
	friend Persistence::SelectionPtrBase<std::unique_ptr<FaceController>>;
	bool persist(Persistence::PersistenceStore & store) override;
	std::string
	getId() const override
	{
		return {};
	};

	void applySingle(ModelFactoryMesh & mesh, const Style::StyleStack & parents, const std::string & name,
			Shape::CreatedFaces & faces) const;
	static std::string getAdjacentFaceName(const ModelFactoryMesh & mesh,
			const std::span<const OpenMesh::FaceHandle> ofrange, OpenMesh::FaceHandle nf);
	Shape::CreatedFaces extrude(ModelFactoryMesh & mesh, const std::string & faceName, OpenMesh::FaceHandle) const;
};
