#include "style.h"
#include "assetFactory.h"

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const Shape::CreatedFaces & faces) const
{
	for (const auto & face : faces) {
		applyStyle(mesh, face.second, getColour(parents));
	}
}

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const ModelFactoryMesh::FaceHandle & face) const
{
	applyStyle(mesh, face, getColour(parents));
}

void
Style::applyStyle(
		ModelFactoryMesh & mesh, const ModelFactoryMesh::FaceHandle & face, EffectiveColour effectiveColour) const
{
	if (effectiveColour.has_value()) {
		mesh.set_color(face, effectiveColour->get());
	}
}

Style::EffectiveColour
Style::getColour(const StyleStack & parents)
{
	return getProperty(parents, &Style::colour, [](auto && style) {
		return style->colour.a > 0;
	});
}

bool
Style::persist(Persistence::PersistenceStore & store)
{
	struct ColourParser : public Persistence::SelectionV<ColourAlpha> {
		using Persistence::SelectionV<ColourAlpha>::SelectionV;
		using Persistence::SelectionV<ColourAlpha>::setValue;
		void
		setValue(std::string && str) override
		{
			if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
				v = mf->parseColour(str);
			}
		}
	};

	return STORE_HELPER(colour, ColourParser) && STORE_MEMBER(texture) && STORE_MEMBER(textureRotation);
}
