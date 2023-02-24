#pragma once

#include "modelFactoryMesh.h"
#include "persistence.h"
#include "shape.h"
#include <string>

class Style {
public:
	using StyleStack = std::vector<const Style *>;

	static ModelFactoryMesh::Color parseColour(const std::string_view &);
	void applyStyle(ModelFactoryMesh &, const StyleStack & parents, const Shape::CreatedFaces &) const;
	void applyStyle(ModelFactoryMesh &, const StyleStack & parents, const ModelFactoryMesh::FaceHandle &) const;

	static std::string_view getProperty(const StyleStack & parents, std::string Style::*member);

	std::string colour;

protected:
	bool persist(Persistence::PersistenceStore & store);
};
