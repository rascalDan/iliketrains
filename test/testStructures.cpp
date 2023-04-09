#include "testStructures.h"

bool
AbsObject::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(base);
}

bool
SubObject::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && AbsObject::persist(store) && STORE_MEMBER(sub);
}

void
SubObject::dummy() const
{
}

std::string
SubObject::getId() const
{
	return "someid";
}

bool
SubObject2::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && AbsObject::persist(store);
}

void
SubObject2::dummy() const
{
}

bool
TestObject::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(flt) && STORE_MEMBER(str) && STORE_MEMBER(bl) && STORE_MEMBER(pos)
			&& STORE_MEMBER(flts) && STORE_MEMBER(poss) && STORE_MEMBER(nest) && STORE_MEMBER(ptr) && STORE_MEMBER(aptr)
			&& STORE_MEMBER(vptr);
}

void
TestObject::postLoad()
{
	postLoadCalled++;
}

bool
SharedTestObject::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(sptr) && STORE_MEMBER(ssptr);
}
