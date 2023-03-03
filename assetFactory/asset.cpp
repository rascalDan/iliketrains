#include "asset.h"

bool
Asset::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(name)
			&& STORE_NAME_HELPER("mesh", meshes, Persistence::Appender<FactoryMesh::Collection>);
}
