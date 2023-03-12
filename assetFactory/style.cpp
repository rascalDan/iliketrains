#include "style.h"
#include "assetFactory.h"

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const Shape::CreatedFaces & faces) const
{
	if (const auto effectiveColour = getProperty(parents, &Style::colour,
				[](auto && style) {
					return style->colour.a > 0;
				});
			effectiveColour.has_value()) {
		for (const auto & face : faces) {
			mesh.set_color(face.second, effectiveColour->get());
		}
	}
}

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const ModelFactoryMesh::FaceHandle & face) const
{
	if (const auto effectiveColour = getProperty(parents, &Style::colour,
				[](auto && style) {
					return style->colour.a > 0;
				});
			effectiveColour.has_value()) {
		mesh.set_color(face, effectiveColour->get());
	}
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
