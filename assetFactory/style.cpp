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
	if (smooth.has_value()) {
		mesh.property(mesh.smoothFaceProperty, face) = smooth.value();
	}
	if (texture.empty()) {
		if (effectiveColour.has_value()) {
			mesh.set_color(face, effectiveColour->get());
		}
	}
	else {
		mesh.set_color(face, {});
		if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
			const auto material = mf->getMaterialIndex(texture);
			mesh.property(mesh.materialFaceProperty, face) = material;
			static constexpr std::array<ModelFactoryTraits::TexCoord2D, 4> coords {{
					{0.F, 0.F},
					{1.F, 0.F},
					{1.F, 1.F},
					{0.F, 1.F},
			}};
			auto coord = coords.begin();
			// Wild assumption that face is a quad and the texture should apply linearly
			for (const auto & heh : mesh.fh_range(face)) {
				mesh.set_texcoord2D(heh, *coord++);
			}
		}
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
	struct ColourParser : public Persistence::SelectionV<RGBA> {
		using Persistence::SelectionV<RGBA>::SelectionV;
		using Persistence::SelectionV<RGBA>::setValue;

		void
		setValue(std::string && str) override
		{
			if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
				v = mf->parseColour(str);
			}
		}
	};

	return STORE_HELPER(colour, ColourParser) && STORE_MEMBER(smooth) && STORE_MEMBER(texture)
			&& STORE_MEMBER(textureRotation);
}
