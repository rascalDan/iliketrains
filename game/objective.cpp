#include "objective.h"
#include "orders.h"

Objective *
Objective::complete() const
{
	return orders->next();
}
