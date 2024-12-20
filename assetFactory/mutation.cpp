#include "mutation.h"
#include <glm/gtx/transform.hpp>
#include <maths.h>

Mutation::Matrix
Mutation::getMatrix() const
{
	return glm::translate(glm::identity<Matrix>(), position) * rotate_ypr<4>(rotation)
			* glm::scale(glm::identity<Matrix>(), scale);
}

Mutation::Matrix
Mutation::getDeformationMatrix() const
{
	return glm::scale(glm::identity<Matrix>(), scale);
}

Mutation::Matrix
Mutation::getLocationMatrix() const
{
	return glm::translate(glm::identity<Matrix>(), position) * rotate_ypr<4>(rotation);
}

bool
Mutation::persist(Persistence::PersistenceStore & store)
{
	return STORE_MEMBER(scale) && STORE_MEMBER(position) && STORE_MEMBER(rotation);
}
