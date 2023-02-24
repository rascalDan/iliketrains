#include "style.h"

ModelFactoryMesh::Color
Style::parseColour(const std::string_view & in)
{
	if (in.empty()) {
		return {};
	}
	if (in[0] == '#') {
		ModelFactoryMesh::Color out {0, 0, 0, 1};
		std::generate_n(out.begin(), (in.length() - 1) / 2, [in = in.data() + 1]() mutable {
			uint8_t channel;
			std::from_chars(in, in + 2, channel, 16);
			in += 2;
			return static_cast<float>(channel) / 256.F;
		});
		return out;
	}
	return {};
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
