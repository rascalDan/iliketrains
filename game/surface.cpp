#include "surface.h"

bool
Surface::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(colorBias) && STORE_MEMBER(quality) && Asset::persist(store);
}
