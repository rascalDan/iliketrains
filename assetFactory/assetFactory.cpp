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
	glm::u8vec3 colour;
	char inname[BUFSIZ];
	while (fscanf(rgb, "%hhu %hhu %hhu %[^\n\r]s", &colour.r, &colour.g, &colour.b, inname) == 4) {
		std::string name {inname};
		normalizeColourName(name);
		out.emplace(std::move(name), colour);
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
