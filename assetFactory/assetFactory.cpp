#include "assetFactory.h"
#include "collections.h"
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
#include <fstream>
#include <numeric>

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

AssetFactory::Assets
AssetFactory::loadAll(const std::filesystem::path & root)
{
	return std::accumulate(std::filesystem::recursive_directory_iterator {root},
			std::filesystem::recursive_directory_iterator {}, Assets {}, [](auto && out, auto && path) {
				if (path.path().extension() == ".xml") {
					out.merge(loadXML(path)->assets);
				}
				return std::move(out);
			});
}

AssetFactory::Colours
AssetFactory::parseX11RGB(const char * path)
{
	std::ifstream rgb {path};
	Colours out;
	while (rgb.good()) {
		Colour colour;
		std::string name;
		rgb >> colour.r >> colour.g >> colour.b;
		std::getline(rgb, name);
		if (rgb.good()) {
			normalizeColourName(name);
			out.emplace(std::move(name), colour / 255.F);
		}
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
		std::generate_n(&out.r, (in.length() - 1) / 2, [in = in.substr(1)]() mutable {
			const auto hexpair = in.substr(0, 2);
			uint8_t channel = 0;
			std::from_chars(hexpair.begin(), hexpair.end(), channel, 16);
			in.remove_prefix(2);
			return static_cast<float>(channel) / 255.F;
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

GLuint
AssetFactory::getMaterialIndex(std::string_view id) const
{
	createTexutre();
	return textureFragmentPositions.at(id);
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
	if (!textureFragments.empty() && !texture) {
		// * layout images
		std::map<const TextureFragment *, std::unique_ptr<const Image>> images;
		std::transform(
				textureFragments.begin(), textureFragments.end(), std::inserter(images, images.end()), [](auto && tf) {
					return decltype(images)::value_type {tf.second.get(), tf.second->image->get()};
				});
		std::vector<TexturePacker::Image> imageSizes;
		std::transform(images.begin(), images.end(), std::back_inserter(imageSizes), [](const auto & i) {
			return TexturePacker::Image {i.second->width, i.second->height};
		});
		const auto [layout, outSize] = TexturePacker {imageSizes}.pack();
		// * create texture
		texture = std::make_shared<TextureAtlas>(outSize.x, outSize.y, layout.size());
		std::transform(images.begin(), images.end(),
				std::inserter(textureFragmentPositions, textureFragmentPositions.end()),
				[position = layout.begin(), size = imageSizes.begin(), this](const auto & i) mutable {
					const auto m = texture->add(*position, *size, i.second->data.data(),
							{
									.wrapU = i.first->mapmodeU,
									.wrapV = i.first->mapmodeV,
							});
					position++;
					size++;
					return decltype(textureFragmentPositions)::value_type {i.first->id, m};
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
