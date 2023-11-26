#pragma once

#include "geometricPlane.h"
#include "modelFactoryMesh_fwd.h"
#include "mutation.h"
#include "persistence.h"
#include "shape.h"
#include "style.h"
#include <map>
#include <string>

class FaceController : public Mutation, public Style, public Persistence::Persistable {
public:
	class Split : public Persistable, public GeometricPlane {
	public:
		std::string id;

	private:
		friend Persistence::SelectionPtrBase<std::unique_ptr<Split>>;
		bool persist(Persistence::PersistenceStore & store) override;

		std::string
		getId() const override
		{
			return {};
		};
	};

	using FaceControllers = std::map<std::string, std::unique_ptr<FaceController>>;
	using Splits = std::map<std::string, std::unique_ptr<Split>>;

	void apply(ModelFactoryMesh & mesh, const Style::StyleStack & parents, const std::string & names,
			Shape::CreatedFaces & faces) const;

	std::string id;
	std::string type;
	FaceControllers faceControllers;
	Splits splits;

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
	Shape::CreatedFaces extrude(ModelFactoryMesh & mesh, const std::string & faceName, OpenMesh::FaceHandle) const;
	Shape::CreatedFaces split(
			ModelFactoryMesh & mesh, const std::string & faceName, OpenMesh::FaceHandle &, const Split &) const;
};
