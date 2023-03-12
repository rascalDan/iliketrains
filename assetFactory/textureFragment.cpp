#include "textureFragment.h"

bool
TextureFragment::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(path);
}
