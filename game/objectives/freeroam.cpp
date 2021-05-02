#include "freeroam.h"
#include <game/activities/go.h>
#include <game/activity.h>
#include <game/network/link.h>
#include <iterator>
#include <memory>
#include <random>
#include <vector>

ActivityPtr
FreeRoam::createActivity() const
{
	return std::make_unique<Go>();
}

Link::Next
FreeRoam::navigate(Link::Nexts::const_iterator begin, Link::Nexts::const_iterator end) const
{
	static std::mt19937 gen(std::random_device {}());
	auto off = std::uniform_int_distribution<long>(0, std::distance(begin, end) - 1)(gen);
	return begin[off];
}
