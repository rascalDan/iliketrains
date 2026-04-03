#include "lights.h"

bool
SpotLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(direction) && STORE_MEMBER(colour) && STORE_MEMBER(kq)
			&& STORE_MEMBER(arc);
}

bool
PointLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(colour) && STORE_MEMBER(kq);
}
