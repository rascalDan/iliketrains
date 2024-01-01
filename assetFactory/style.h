#pragma once

#include "modelFactoryMesh.h"
#include "persistence.h"
#include "shape.h"
#include <optional>
#include <string>
#include <utility>

class Style {
public:
	using StyleStack = std::vector<const Style *>;
	using EffectiveColour = std::optional<std::reference_wrapper<const RGBA>>;

	void applyStyle(ModelFactoryMesh &, const StyleStack & parents, const Shape::CreatedFaces &) const;
	void applyStyle(ModelFactoryMesh &, const StyleStack & parents, const ModelFactoryMesh::FaceHandle &) const;

	template<typename T>
	static std::optional<std::reference_wrapper<const T>>
	getProperty(const StyleStack & parents, T Style::*member, auto && test)
	{
		if (const auto itr = std::find_if(parents.rbegin(), parents.rend(), std::forward<decltype(test)>(test));
				itr != parents.rend()) {
			return (*itr)->*member;
		}
		return {};
	}

	static EffectiveColour getColour(const StyleStack & parents);

	RGBA colour {};
	std::optional<bool> smooth;
	std::string texture;
	std::string textureRotation; // Multiples of 90deg, no int/enum support

protected:
	bool persist(Persistence::PersistenceStore & store);
	void applyStyle(ModelFactoryMesh &, const ModelFactoryMesh::FaceHandle &, EffectiveColour) const;
};
