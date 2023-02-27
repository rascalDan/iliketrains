#include "style.h"
#include "assetFactory.h"

ModelFactoryMesh::Color
Style::parseColour(const std::string_view & in)
{
	if (in.empty()) {
		throw std::runtime_error("Empty colour specification");
	}
	if (in[0] == '#') {
		if (in.length() > 9 || in.length() % 2 == 0) {
			throw std::runtime_error("Invalid hex colour specification");
		}
		ModelFactoryMesh::Color out {0, 0, 0, 1};
		std::generate_n(out.begin(), (in.length() - 1) / 2, [in = in.data() + 1]() mutable {
			uint8_t channel;
			std::from_chars(in, in + 2, channel, 16);
			in += 2;
			return static_cast<float>(channel) / 256.F;
		});
		return out;
	}
	if (auto mf = std::dynamic_pointer_cast<const AssetFactory>(Persistence::sharedObjects.at("assetFactory"))) {
		if (const auto colour = mf->colours.find(in); colour != mf->colours.end()) {
			const auto out = glm::vec3 {colour->second} / 256.F;
			return {out.r, out.g, out.b, 1.f};
		}
	}
	throw std::runtime_error("No such asset factory colour");
}

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const Shape::CreatedFaces & faces) const
{
	if (const auto effectiveColour = getProperty(parents, &Style::colour); !effectiveColour.empty()) {
		const auto parsedColour = parseColour(effectiveColour);
		for (const auto & face : faces) {
			mesh.set_color(face.second, parsedColour);
		}
	}
}

void
Style::applyStyle(ModelFactoryMesh & mesh, const StyleStack & parents, const ModelFactoryMesh::FaceHandle & face) const
{
	if (const auto effectiveColour = getProperty(parents, &Style::colour); !effectiveColour.empty()) {
		const auto parsedColour = parseColour(effectiveColour);
		mesh.set_color(face, parsedColour);
	}
}

std::string_view
Style::getProperty(const StyleStack & parents, std::string Style::*member)
{
	if (const auto itr = std::find_if(parents.rbegin(), parents.rend(),
				[&member](auto && s) {
					return !(s->*member).empty();
				});
			itr != parents.rend()) {
		return (*itr)->*member;
	}
	return {};
}

bool
Style::persist(Persistence::PersistenceStore & store)
{
	return STORE_MEMBER(colour);
}
