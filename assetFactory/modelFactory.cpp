#include "modelFactory.h"
#include "cuboid.h"
#include "cylinder.h"
#include "modelFactoryMesh_fwd.h"
#include "object.h"
#include "plane.h"
#include "saxParse-persistence.h"
#include <filesystem.h>

ModelFactory::ModelFactory() :
	shapes {
			{"plane", std::make_shared<Plane>()},
			{"cuboid", std::make_shared<Cuboid>()},
			{"cylinder", std::make_shared<Cylinder>()},
	}
{
}

std::shared_ptr<ModelFactory>
ModelFactory::loadXML(const std::filesystem::path & filename)
{
	filesystem::FileStar file {filename.c_str(), "r"};
	return Persistence::SAXParsePersistence {}.loadState<std::shared_ptr<ModelFactory>>(file);
}

bool
ModelFactory::persist(Persistence::PersistenceStore & store)
{
	using MapObjects = Persistence::MapByMember<Shapes, Object>;
	return STORE_TYPE && STORE_NAME_HELPER("object", shapes, MapObjects);
}
