#include "assetFactory.h"
#include "collections.hpp"
#include "cuboid.h"
#include "cylinder.h"
#include "filesystem.h"
#include "gfx/image.h"
#include "gfx/models/texture.h"
#include "modelFactoryMesh_fwd.h"
#include "object.h"
#include "plane.h"
#include "resource.h"
#include "saxParse-persistence.h"
#include "texturePacker.h"

AssetFactory::AssetFactory() :
	shapes {
			{"plane", std::make_shared<Plane>()},
			{"cuboid", std::make_shared<Cuboid>()},
			{"cylinder", std::make_shared<Cylinder>()},
	},
	colours {parseX11RGB("/usr/share/X11/rgb.txt")}
{
}

std::shared_ptr<AssetFactory>
AssetFactory::loadXML(const std::filesystem::path & filename)
{
	filesystem::FileStar file {filename.c_str(), "r"};
	return Persistence::SAXParsePersistence {}.loadState<std::shared_ptr<AssetFactory>>(file);
}

AssetFactory::Colours
AssetFactory::parseX11RGB(const char * path)
{
	filesystem::FileStar rgb {path, "r"};
	Colours out;
	Colour colour;
	char inname[BUFSIZ];
	while (fscanf(rgb, "%f %f %f %[^\n\r]s", &colour.r, &colour.g, &colour.b, inname) == 4) {
		std::string name {inname};
		normalizeColourName(name);
		out.emplace(std::move(name), colour / 255.f);
	}
	return out;
}

void
AssetFactory::normalizeColourName(std::string & name)
{
	std::erase_if(name, ::isblank);
	name *= [l = std::locale {}](auto & ch) {
		ch = std::tolower(ch, l);
	};
}

AssetFactory::ColourAlpha
AssetFactory::parseColour(std::string_view in) const
{
	if (in.empty()) {
		throw std::runtime_error("Empty colour specification");
	}
	if (in[0] == '#') {
		if (in.length() > 9 || in.length() % 2 == 0) {
			throw std::runtime_error("Invalid hex colour specification");
		}
		ColourAlpha out {0, 0, 0, 1};
		std::generate_n(&out.r, (in.length() - 1) / 2, [in = in.data() + 1]() mutable {
			uint8_t channel;
			std::from_chars(in, in + 2, channel, 16);
			in += 2;
			return static_cast<float>(channel) / 255.f;
		});
		return out;
	}
	if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
		if (const auto colour = mf->colours.find(in); colour != mf->colours.end()) {
			return {colour->second, 1};
		}
	}
	throw std::runtime_error("No such asset factory colour");
}

AssetFactory::TextureFragmentCoords
AssetFactory::getTextureCoords(std::string_view id) const
{
	createTexutre();
	const auto & fragmentUV = textureFragmentPositions.at(id);
	return {
			fragmentUV.xy(),
			fragmentUV.zy(),
			fragmentUV.zw(),
			fragmentUV.xw(),
	};
}

Asset::TexturePtr
AssetFactory::getTexture() const
{
	createTexutre();
	return texture;
}

void
AssetFactory::createTexutre() const
{
	if (!textureFragments.empty() && (!texture || textureFragmentPositions.empty())) {
		// * layout images
		std::vector<TexturePacker::Image> imageSizes;
		std::transform(
				textureFragments.begin(), textureFragments.end(), std::back_inserter(imageSizes), [](const auto & tf) {
					return TexturePacker::Image {tf.second->image->width, tf.second->image->height};
				});
		const auto [layout, outSize] = TexturePacker {imageSizes}.pack();
		// * create texture
		texture = std::make_shared<Texture>(outSize.x, outSize.y, TextureOptions {.wrap = GL_CLAMP_TO_EDGE});
		std::transform(textureFragments.begin(), textureFragments.end(),
				std::inserter(textureFragmentPositions, textureFragmentPositions.end()),
				[position = layout.begin(), size = imageSizes.begin(), outSize = glm::vec2 {outSize}](
						const auto & tf) mutable {
					const auto positionFraction = glm::vec4 {*position, *position + *size} / outSize.xyxy();
					glTexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(position->x), static_cast<GLint>(position->y),
							static_cast<GLint>(size->x), static_cast<GLint>(size->y), GL_RGBA, GL_UNSIGNED_BYTE,
							tf.second->image->data.data());
					position++;
					size++;
					return decltype(textureFragmentPositions)::value_type {tf.first, positionFraction};
				});
	}
}

bool
AssetFactory::persist(Persistence::PersistenceStore & store)
{
	using MapObjects = Persistence::MapByMember<Shapes, std::shared_ptr<Object>>;
	using MapAssets = Persistence::MapByMember<Assets>;
	using MapTextureFragments = Persistence::MapByMember<TextureFragments>;
	using MapAssImp = Persistence::MapByMember<AssImps, std::shared_ptr<AssImp>, &AssImp::path>;
	return STORE_TYPE && STORE_NAME_HELPER("object", shapes, MapObjects)
			&& STORE_NAME_HELPER("textureFragment", textureFragments, MapTextureFragments)
			&& STORE_NAME_HELPER("assimp", assimps, MapAssImp) && STORE_NAME_HELPER("asset", assets, MapAssets);
}
