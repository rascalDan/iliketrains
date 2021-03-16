#include "goto.h"
#include <algorithm>
#include <game/activities/go.h>
#include <game/activity.h>
#include <game/network/link.h>
#include <game/network/routeWalker.h>
#include <game/objective.h>
#include <memory>
#include <numeric>
#include <vector>

GoTo::GoTo(Orders * o, const Link::End & cp, float d, const NodePtr & dest) :
	Objective(o), links(RouteWalker().findRouteTo(cp, dest)), startDist {d}
{
}

ActivityPtr
GoTo::createActivity() const
{
	return std::make_unique<Go>(std::accumulate(links.begin(), links.end(), 0,
										[](auto p, const auto & l) {
											return p += l.first.lock()->length;
										})
			+ startDist

	);
}

inline bool
operator==(const Link::Next & l, const Link::Next & n)
{
	return l.second == n.second && l.first.lock() == n.first.lock();
}

Link::Next
GoTo::navigate(Link::Nexts::const_iterator begin, Link::Nexts::const_iterator end) const
{
	const auto nextStep = std::find_first_of(links.begin(), links.end(), begin, end);
	if (nextStep == links.end()) {
		return *begin;
	}
	return *nextStep;
}
