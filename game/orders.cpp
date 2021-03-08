#include "orders.h"
#include <algorithm>
#include <vector>

Objective *
Orders::current() const
{
	return objects.front().get();
}

Objective *
Orders::next()
{
	std::rotate(objects.begin(), objects.begin() + 1, objects.end());
	return current();
}
