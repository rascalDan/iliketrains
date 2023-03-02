#include "assetFactory.h"
#include "collections.hpp"
#include "cuboid.h"
#include "cylinder.h"
#include "modelFactoryMesh_fwd.h"
#include "object.h"
#include "plane.h"
#include "saxParse-persistence.h"
#include <filesystem.h>

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
	if (auto mf = std::dynamic_pointer_cast<const AssetFactory>(Persistence::sharedObjects.at("assetFactory"))) {
		if (const auto colour = mf->colours.find(in); colour != mf->colours.end()) {
			return {colour->second, 1};
		}
	}
	throw std::runtime_error("No such asset factory colour");
}
bool
AssetFactory::persist(Persistence::PersistenceStore & store)
{
	using MapObjects = Persistence::MapByMember<Shapes, std::shared_ptr<Object>>;
	using MapAssets = Persistence::MapByMember<Assets>;
	return STORE_TYPE && STORE_NAME_HELPER("object", shapes, MapObjects)
			&& STORE_NAME_HELPER("asset", assets, MapAssets);
}

bool
Asset::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(name)
			&& STORE_NAME_HELPER("mesh", meshes, Persistence::Appender<FactoryMesh::Collection>);
}
