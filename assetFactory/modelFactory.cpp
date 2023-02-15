#include "modelFactory.h"
#include "assetFactoryConfig.h"
#include "cuboid.h"
#include "cylinder.h"
#include "plane.h"

ModelFactory::ModelFactory() :
	shapes {
			{"plane", std::make_shared<Plane>()},
			{"cuboid", std::make_shared<Cuboid>()},
			{"cylinder", std::make_shared<Cylinder>()},
	}
{
}
